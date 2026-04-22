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
// [수정] startBroadcasting: 닉네임(hostName)을 추가로 받아 저장합니다.
// =========================================================================
void NetworkManager::startBroadcasting(QString roomName, int tcpPort, QString hostName) {
    udpBroadcastSocket = new QUdpSocket(this);
    broadcastTimer = new QTimer(this);
    currentRoomName = roomName;
    currentTcpPort = tcpPort;
    currentHostName = hostName; // 헤더에 추가한 변수에 저장

    connect(broadcastTimer, &QTimer::timeout, this, &NetworkManager::sendBroadcast);
    broadcastTimer->start(1000);
}

void NetworkManager::sendBroadcast() {
    // 패킷 형태: "CHESS_LAN|방이름|TCP포트|닉네임" (4개로 확장)
    QString msg = QString("CHESS_LAN|%1|%2|%3").arg(currentRoomName).arg(currentTcpPort).arg(currentHostName);
    QByteArray datagram = msg.toUtf8();

    udpBroadcastSocket->writeDatagram(datagram, QHostAddress::Broadcast, 45454);
}

void NetworkManager::stopBroadcasting() {
    if (broadcastTimer) { broadcastTimer->stop(); broadcastTimer->deleteLater(); broadcastTimer = nullptr; }
    if (udpBroadcastSocket) { udpBroadcastSocket->deleteLater(); udpBroadcastSocket = nullptr; }
}

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

// =========================================================================
// [수정] processPendingDatagrams: 닉네임까지 추출해서 4개의 인자를 emit 합니다.
// =========================================================================
void NetworkManager::processPendingDatagrams() {
    while (udpListenSocket && udpListenSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpListenSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        udpListenSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        QString data = QString::fromUtf8(datagram);
        QStringList parts = data.split("|");

        // 닉네임이 포함되었으므로 조각이 4개 이상이어야 합니다.
        if (parts.size() >= 4 && parts[0] == "CHESS_LAN") {
            QString roomName = parts[1];
            int tcpPort = parts[2].toInt();
            QString hostName = parts[3]; // 닉네임 조각 추출

            QString ip = sender.toString();
            if (ip.startsWith("::ffff:")) ip = ip.mid(7);

            // [에러 해결!] 이제 4개의 인자(ip, port, roomName, hostName)를 모두 보냅니다.
            emit gameDiscovered(ip, tcpPort, roomName, hostName);
        }
    }
}