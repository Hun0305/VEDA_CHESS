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

private:
    NetworkManager *networkManager;

    QTextEdit *logWindow; // 로그를 출력할 위젯
    QString getChessNotation(BoardPosition pos); // 좌표를 A1, D4 형식으로 변환

    BoardViewModel boardViewModel;
    bool gameStarted;
    BoardView *board;
    PlayerView *blackPlayerView;
    PlayerView *whitePlayerView;

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
};

#endif // GAME_H
