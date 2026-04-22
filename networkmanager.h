#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket> // 추가
#include <QTimer>     // 추가

class NetworkManager : public QObject {
    Q_OBJECT
public:
    explicit NetworkManager(QObject *parent = nullptr);

    bool startHosting(int port = 12345);      // 호스트 시작
    void connectToHost(QString ip, int port); // 서버 접속
    void sendMove(QString moveData);         // 데이터 전송

    // --- 새로 추가할 UDP 브로드캐스트용 함수 ---
    void startBroadcasting(QString roomName, int tcpPort);
    void stopBroadcasting();
    void startListeningForGames();
    void stopListeningForGames();

signals:
    void connected();                        // 연결 완료 시그널
    void dataReceived(QString data);         // 데이터 수신 시그널

    // --- 새로 추가할 시그널 (방을 찾았을 때 UI로 전달) ---
    void gameDiscovered(QString ip, int port, QString roomName);

private slots:
    void onNewConnection();                  // 호스트: 새 플레이어 접속 시
    void onReadyRead();                      // 공통: 데이터 수신 시

    void sendBroadcast();
    void processPendingDatagrams();

private:
    QTcpServer *tcpServer;
    QTcpSocket *tcpSocket;

    QUdpSocket *udpBroadcastSocket = nullptr;
    QUdpSocket *udpListenSocket = nullptr;
    QTimer *broadcastTimer = nullptr;
    QString currentRoomName;
    int currentTcpPort;
};

#endif // NETWORKMANAGER_H

