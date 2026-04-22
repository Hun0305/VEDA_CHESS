#ifndef GAME_H
#define GAME_H

#include <QObject>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMouseEvent>
#include "boardview.h"
#include "boardviewmodel.h"
#include "pawnfield.h"
#include "playerview.h"
#include "networkmanager.h"
#include <QTextEdit>
#include <QGraphicsProxyWidget>
#include <QTime>
#include <QLineEdit>
#include <QComboBox>

class GameView : public QGraphicsView {
    Q_OBJECT

public:
    GameView();
    QGraphicsScene *scene;



public slots:
    void displayRoomList();
    void displayMainMenu();
    void hostGame();
    void startGame();
    void quitGame();
    void resignGame(); // 기존 resetGame() 대신 사용할 함수
    void onDataReceived(QString data); // 네트워크 데이터 처리 슬롯
    void addLog(QString message); // 로그 메시지 추가 함수
    void globalHostGame();
    void globalJoinGame();
    void setLoggedInUser(QString id) { loggedInUserId = id; } // 아이디 설정 함수
    void showHostGameSettings(); // 방 생성 설정 화면 띄우기
    void confirmHostGame();      // 설정 완료 후 호스팅 시작
    void onGameDiscovered(QString ip, int port, QString roomName, QString hostName);

private:
    NetworkManager *networkManager;

    int nextRoomY;               // 다음 방이 그려질 Y 좌표
    QSet<QString> discoveredRooms; // 중복 방 표시 방지 (IP:Port 저장)

    QTextEdit *logWindow; // 로그를 출력할 위젯
    QString getChessNotation(BoardPosition pos); // 좌표를 A1, D4 형식으로 변환

    QLineEdit *roomNameInput;
    QComboBox *networkModeInput;
    QLineEdit *portInput;

    BoardViewModel boardViewModel;
    bool gameStarted;
    BoardView *board;
    PlayerView *blackPlayerView;
    PlayerView *whitePlayerView;
    QString loggedInUserId; // 로그인한 유저의 ID를 저장

    PlayerType myColor; // 내 색상 저장 (호스트=White, 조인=Black)
    void executeMove(BoardPosition from, BoardPosition to); // 실제 이동 로직 분리

    void drawBoard();
    void drawSettingsPanel();
    void drawUserPanel();
    PlayerView* drawViewForUser(PlayerType player);
    void drawTitle(double yPosition, int fontSize);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void selectPawn(PawnField *pawn);
    void handleSelectingPointForActivePawnByMouse(QPoint point);
    void setCheckStateOnPlayerView(PlayerType player, bool isInCheck);
    void moveActivePawnToSelectedPoint(QPoint point);
    void releaseActivePawn();
    void showCongratulationsScreen(PlayerType winner);
    void updateDatabaseResult(bool isWinner);
};

#endif // GAME_H
