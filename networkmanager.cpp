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
        tcpSocket->write(moveData.toUtf8());
    }
}