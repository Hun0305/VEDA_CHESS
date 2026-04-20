#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

class NetworkManager : public QObject {
    Q_OBJECT
public:
    explicit NetworkManager(QObject *parent = nullptr);

    bool startHosting(int port = 12345);      // 호스트 시작
    void connectToHost(QString ip, int port); // 서버 접속
    void sendMove(QString moveData);         // 데이터 전송

signals:
    void connected();                        // 연결 완료 시그널
    void dataReceived(QString data);         // 데이터 수신 시그널

private slots:
    void onNewConnection();                  // 호스트: 새 플레이어 접속 시
    void onReadyRead();                      // 공통: 데이터 수신 시

private:
    QTcpServer *tcpServer;
    QTcpSocket *tcpSocket;
};

#endif // NETWORKMANAGER_H
