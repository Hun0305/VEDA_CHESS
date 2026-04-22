#include "networkmanager.h"
#include <QTimer>
#include <QNetworkDatagram>

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent), tcpServer(nullptr), tcpSocket(nullptr),
    udpBroadcastSocket(nullptr), udpListenSocket(nullptr), broadcastTimer(nullptr) {}

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
    emit connected();
}

void NetworkManager::onReadyRead() {
    QByteArray data = tcpSocket->readAll();
    emit dataReceived(QString::fromUtf8(data));
}

void NetworkManager::sendMove(QString moveData) {
    if (tcpSocket && tcpSocket->isOpen()) {
        QString framedData = moveData + ";";
        tcpSocket->write(framedData.toUtf8());
        tcpSocket->flush();
    }
}

// =========================================================================
// [수정 완료] startBroadcasting: 닉네임(hostName) 인자를 추가하여 패킷을 보냅니다.
// =========================================================================
void NetworkManager::startBroadcasting(QString roomName, int tcpPort, QString hostName) {
    udpBroadcastSocket = new QUdpSocket(this);
    broadcastTimer = new QTimer(this);
    currentRoomName = roomName;
    currentTcpPort = tcpPort;
    currentHostName = hostName; // 헤더에 선언된 변수에 저장

    connect(broadcastTimer, &QTimer::timeout, this, &NetworkManager::sendBroadcast);
    broadcastTimer->start(1000);
}

void NetworkManager::sendBroadcast() {
    // 패킷 형태 수정: "CHESS_LAN|방이름|TCP포트|닉네임" (4단 구성)
    QString msg = QString("CHESS_LAN|%1|%2|%3").arg(currentRoomName).arg(currentTcpPort).arg(currentHostName);
    QByteArray datagram = msg.toUtf8();
    udpBroadcastSocket->writeDatagram(datagram, QHostAddress::Broadcast, 45454);
}

void NetworkManager::stopBroadcasting() {
    if (broadcastTimer) { broadcastTimer->stop(); broadcastTimer->deleteLater(); broadcastTimer = nullptr; }
    if (udpBroadcastSocket) { udpBroadcastSocket->deleteLater(); udpBroadcastSocket = nullptr; }
}

// =========================================================================
// [수정 완료] 로비 리스닝: 받은 패킷에서 닉네임까지 추출해서 UI에 전달합니다.
// =========================================================================
void NetworkManager::startListeningForGames() {
    if (!udpListenSocket) {
        udpListenSocket = new QUdpSocket(this);
        udpListenSocket->bind(QHostAddress::AnyIPv4, 45454, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
        connect(udpListenSocket, &QUdpSocket::readyRead, this, &NetworkManager::processPendingDatagrams);
    }
}

void NetworkManager::stopListeningForGames() {
    if (udpListenSocket) { udpListenSocket->deleteLater(); udpListenSocket = nullptr; }
}

void NetworkManager::processPendingDatagrams() {
    while (udpListenSocket && udpListenSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpListenSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        udpListenSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        QString data = QString::fromUtf8(datagram);
        QStringList parts = data.split("|");

        // [에러 해결 핵심] parts.size()가 4인 경우(닉네임 포함)를 처리합니다.
        if (parts.size() >= 4 && parts[0] == "CHESS_LAN") {
            QString roomName = parts[1];
            int tcpPort = parts[2].toInt();
            QString hostName = parts[3]; // 4번째 데이터인 닉네임 추출

            QString ip = sender.toString();
            if (ip.startsWith("::ffff:")) ip = ip.mid(7);

            // [에러 해결!] 헤더에 약속한 대로 4개의 정보를 모두 쏴줍니다.
            emit gameDiscovered(ip, tcpPort, roomName, hostName);
        }
    }
}