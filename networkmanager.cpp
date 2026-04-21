#include "networkmanager.h"

NetworkManager::NetworkManager(QObject *parent) : QObject(parent), tcpServer(nullptr), tcpSocket(nullptr) {}

bool NetworkManager::startHosting(int port) {
    tcpServer = new QTcpServer(this);
    connect(tcpServer, &QTcpServer::newConnection, this, &NetworkManager::onNewConnection);
    return tcpServer->listen(QHostAddress::Any, port);
}

void NetworkManager::connectToHost(QString ip, int port) {
    tcpSocket = new QTcpSocket(this);
    connect(tcpSocket, &QTcpSocket::connected, this, &NetworkManager::connected);
    connect(tcpSocket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    tcpSocket->connectToHost(ip, port);
}

void NetworkManager::onNewConnection() {
    tcpSocket = tcpServer->nextPendingConnection();
    connect(tcpSocket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    emit connected(); // 호스트도 연결된 것으로 간주
}

void NetworkManager::onReadyRead() {
    QByteArray data = tcpSocket->readAll();
    emit dataReceived(QString(data));
}

void NetworkManager::sendMove(QString moveData) {
    if (tcpSocket && tcpSocket->isOpen()) {
        // 메시지 끝에 구분자 ';'를 추가하여 전송
        QString framedData = moveData + ";";
        tcpSocket->write(framedData.toUtf8());
        tcpSocket->flush();
        qDebug() << "Sending Packet:" << framedData;
    }
}

// ==========================================
// UDP 호스트: 내 방 정보를 1초마다 외침
// ==========================================
void NetworkManager::startBroadcasting(QString roomName, int tcpPort) {
    udpBroadcastSocket = new QUdpSocket(this);
    broadcastTimer = new QTimer(this);
    currentRoomName = roomName;
    currentTcpPort = tcpPort;

    // 1초(1000ms)마다 sendBroadcast 함수 실행
    connect(broadcastTimer, &QTimer::timeout, this, &NetworkManager::sendBroadcast);
    broadcastTimer->start(1000);
}

void NetworkManager::sendBroadcast() {
    // 패킷 형태: "CHESS_LAN|방이름|TCP포트"
    QString msg = QString("CHESS_LAN|%1|%2").arg(currentRoomName).arg(currentTcpPort);
    QByteArray datagram = msg.toUtf8();

    // 같은 네트워크 상의 모든 PC의 45454 포트로 메시지를 던짐
    udpBroadcastSocket->writeDatagram(datagram, QHostAddress::Broadcast, 45454);
}

void NetworkManager::stopBroadcasting() {
    if (broadcastTimer) { broadcastTimer->stop(); broadcastTimer->deleteLater(); broadcastTimer = nullptr; }
    if (udpBroadcastSocket) { udpBroadcastSocket->deleteLater(); udpBroadcastSocket = nullptr; }
}

// ==========================================
// UDP 클라이언트 (Lobby): 다른 방의 외침을 들음
// ==========================================
void NetworkManager::startListeningForGames() {
    if (!udpListenSocket) {
        udpListenSocket = new QUdpSocket(this);
        // 45454 포트로 들어오는 브로드캐스트를 듣기 위해 바인딩
        udpListenSocket->bind(QHostAddress::AnyIPv4, 45454, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
        connect(udpListenSocket, &QUdpSocket::readyRead, this, &NetworkManager::processPendingDatagrams);
    }
}

void NetworkManager::stopListeningForGames() {
    if (udpListenSocket) { udpListenSocket->deleteLater(); udpListenSocket = nullptr; }
}

void NetworkManager::processPendingDatagrams() {
    while (udpListenSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpListenSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        udpListenSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        QString data = QString::fromUtf8(datagram);
        QStringList parts = data.split("|");

        // 올바른 체스 게임 브로드캐스트인지 확인
        if (parts.size() == 3 && parts[0] == "CHESS_LAN") {
            QString roomName = parts[1];
            int tcpPort = parts[2].toInt();

            // IPv4 주소 깔끔하게 파싱 (::ffff:192.168.x.x 형태 방지)
            QString ip = sender.toString();
            if (ip.startsWith("::ffff:")) ip = ip.mid(7);

            // UI쪽에 방을 찾았다고 알림
            emit gameDiscovered(ip, tcpPort, roomName);
        }
    }
}