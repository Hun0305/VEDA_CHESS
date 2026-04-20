#include "gameview.h"
#include <QGraphicsTextItem>
#include <QColor>
#include <QBrush>
#include "actionbutton.h"
#include "congratulationsview.h"
#include "constants.h"
#include "utils.h"

int viewWidth = 1200;
int viewHeight= 768;

GameView::GameView() {

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFixedSize(viewWidth, viewHeight);

    scene = new QGraphicsScene();
    scene->setSceneRect(0, 0, viewWidth, viewHeight);
    setScene(scene);

    QBrush brush;
    brush.setStyle((Qt::SolidPattern));
    QColor color = QColor(44, 41, 51);
    brush.setColor(color);
    scene->setBackgroundBrush(brush);

    gameStarted = false;
}

void GameView::displayMainMenu() {
    scene->clear();

    // create title label
    double titleYPosition = 150;
    drawTitle(titleYPosition, 50);

    // create start button
    ActionButton *startButton = new ActionButton("Play");
    double buttonXPosition = this->width()/2 - startButton->boundingRect().width()/2;
    double buttonYPosition = 275;
    startButton->setPos(buttonXPosition, buttonYPosition);

    connect(startButton, SIGNAL(buttonPressed()), this, SLOT(displayRoomList()));
    scene->addItem(startButton);

    // create quit button
    ActionButton *quitButton = new ActionButton("Quit");
    double quitXPosition = this->width()/2 - quitButton->boundingRect().width()/2;
    double quitYPosition = 350;
    quitButton->setPos(quitXPosition, quitYPosition);

    connect(quitButton, SIGNAL(buttonPressed()), this, SLOT(quitGame()));
    scene->addItem(quitButton);
}

void GameView::displayRoomList() {
    scene->clear(); // 현재 화면 지우기

    // 1. 제목 그리기
    drawTitle(50, 40);

    // 2. 테이블 헤더 설정 (간격 조절)
    int startY = 150;
    int col1 = 200, col3 = 700, col4 = 900; // X 좌표 간격

    QGraphicsTextItem *h1 = Utils::createTextItem("Game Name", 20, Qt::white);
    h1->setPos(col1, startY);
    scene->addItem(h1);

    QGraphicsTextItem *h2 = Utils::createTextItem("Players", 20, Qt::white);
    h2->setPos(col3, startY);
    scene->addItem(h2);

    QGraphicsTextItem *h3 = Utils::createTextItem("Action", 20, Qt::white);
    h3->setPos(col4, startY);
    scene->addItem(h3);

    // 3. 더미 방 목록 데이터 루프 (예시 3개)
    struct Room { QString name; QString players; };
    QList<Room> rooms = { {"Software Chess", "1/2"}, {"PVP Challenge", "1/2"}, {"Wait for King", "0/2"} };

    for(int i = 0; i < rooms.size(); ++i) {
        int rowY = startY + 60 + (i * 70); // 행 간격

        // 방 이름
        QGraphicsTextItem *rName = Utils::createTextItem(rooms[i].name, 18, Qt::lightGray);
        rName->setPos(col1, rowY + 10);
        scene->addItem(rName);

        // 인원 수
        QGraphicsTextItem *rPlayers = Utils::createTextItem(rooms[i].players, 18, Qt::lightGray);
        rPlayers->setPos(col3, rowY + 10);
        scene->addItem(rPlayers);

        // Join 버튼 (기존 ActionButton 재활용)
        ActionButton *joinBtn = new ActionButton("Join");
        joinBtn->setPos(col4, rowY);
        joinBtn->setScale(0.7); // 리스트용으로 조금 작게 조절
        connect(joinBtn, &ActionButton::buttonPressed, this, [this](){
            networkManager = new NetworkManager(this);
            networkManager->connectToHost("127.0.0.1", 12345); // 로컬 주소 접속

            connect(networkManager, &NetworkManager::connected, this, &GameView::startGame);
        });
        scene->addItem(joinBtn);
    }

    // 4. 뒤로가기 버튼
    ActionButton *hostButton = new ActionButton("Host Game");
    // 위치를 "Back to Menu" 버튼 옆으로 잡습니다.
    hostButton->setPos(this->width()/2 - hostButton->boundingRect().width() - 20, 650);
    connect(hostButton, SIGNAL(buttonPressed()), this, SLOT(hostGame()));
    scene->addItem(hostButton);

    // --- 기존: Back to Menu 버튼 (위치 살짝 조정) ---
    ActionButton *backButton = new ActionButton("Back to Menu");
    backButton->setPos(this->width()/2 + 20, 650);
    connect(backButton, SIGNAL(buttonPressed()), this, SLOT(displayMainMenu()));
    scene->addItem(backButton);
}

// 방 만들기 버튼 클릭 시 실행될 함수
void GameView::hostGame() {
    networkManager = new NetworkManager(this);
    if (networkManager->startHosting()) {
        qDebug() << "Server started! Waiting for player...";
        // 연결될 때까지 대기 메시지 표시 (UI 작업)
        connect(networkManager, &NetworkManager::connected, this, [this](){
            startGame(); // 상대방 접속 시 게임 시작
        });
    }
}

void GameView::startGame() {

    scene->clear();

    boardViewModel = BoardViewModel();

    drawBoard();
    drawSettingsPanel();
    drawUserPanel();
    int titleYPosition = Constants::defaultMargin;
    drawTitle(titleYPosition, 40);
    gameStarted = true;
}

void GameView::quitGame() {
    close();
}

void GameView::resetGame() {
    gameStarted = false;
    scene->clear();
    startGame();
}

void GameView::drawBoard() {
    board = new BoardView();
    board->draw();
    board->initializePawnFields(boardViewModel.getBlackPawns());
    board->initializePawnFields(boardViewModel.getWhitePawns());
}

void GameView::drawSettingsPanel() {
    // create quit button
    ActionButton *resetButton = new ActionButton("Reset game");
    double resetXPosition = 690 + resetButton->boundingRect().width()/2;
    double resetYPosition = 420;
    resetButton->setPos(resetXPosition, resetYPosition);

    connect(resetButton, SIGNAL(buttonPressed()), this, SLOT(resetGame()));
    scene->addItem(resetButton);

    // create quit button
    ActionButton *quitButton = new ActionButton("Quit game");
    double quitXPosition = 690 + quitButton->boundingRect().width()/2;
    double quitYPosition = 490;
    quitButton->setPos(quitXPosition, quitYPosition);

    connect(quitButton, SIGNAL(buttonPressed()), this, SLOT(quitGame()));
    scene->addItem(quitButton);
}

void GameView::drawUserPanel() {
    blackPlayerView = drawViewForUser(PlayerType::black);
    whitePlayerView = drawViewForUser(PlayerType::white);

    blackPlayerView->setActive(true);
}

PlayerView* GameView::drawViewForUser(PlayerType player) {
    PlayerView *playerView = new PlayerView();

    int xPosition = 80;
    int yPosition = BoardView::startYPosition;

    switch (player) {
    case PlayerType::black:
        xPosition = 680;
        break;
    case PlayerType::white:
        xPosition = 680 + PlayerView::defaultWidthHeight + 20;
        break;
    }

    scene->addItem(playerView);
    playerView->setRect(xPosition, yPosition, PlayerView::defaultWidthHeight, PlayerView::defaultWidthHeight);
    playerView->setPlayer(player);

    return playerView;
}

void GameView::drawTitle(double yPosition, int fontSize) {
    QGraphicsTextItem *title = Utils::createTextItem("Chess Game", fontSize, Qt::white);
    double xPosition = this->width()/2 - title->boundingRect().width()/2;
    title->setPos(xPosition, yPosition);
    scene->addItem(title);
}

void GameView::mousePressEvent(QMouseEvent *event) {
    if (!gameStarted) {
        QGraphicsView::mousePressEvent(event);
        return;
    } else if (event->button() == Qt::RightButton) {
        releaseActivePawn();
    } else if (boardViewModel.getActivePawn()) {
        handleSelectingPointForActivePawnByMouse(event->pos());
    } else {
        PawnField *pawn = board->getPawnAtMousePosition(event->pos());
        selectPawn(pawn);
    }

    QGraphicsView::mousePressEvent(event);
}

void GameView::mouseMoveEvent(QMouseEvent *event) {
    // if there is a pawn selected, then make it follow the mouse
    if (gameStarted && boardViewModel.getActivePawn()) {
        board->moveActivePawnToMousePosition(event->pos(), boardViewModel.getActivePawn());
    }

    QGraphicsView::mouseMoveEvent(event);
}


void GameView::selectPawn(PawnField *pawn) {
    if (pawn == nullptr) {
        return;
    }

    boardViewModel.setActivePawnForField(pawn);
}

void GameView::handleSelectingPointForActivePawnByMouse(QPoint point) {
    if (boardViewModel.getActivePawn() == nullptr) {
        return;
    }

    // check if mouse selected place on board
    if (!boardViewModel.validatePawnPalcementForMousePosition(point)) {
        return;
    }

    BoardPosition boardPosition = boardViewModel.getBoardPositionForMousePosition(point);

    // first validate Move
    if (!boardViewModel.validatePawnMove(boardPosition)) {
        return;
    }

    // Players cannot make any move that places their own king in check
    bool isKingInCheck = boardViewModel.isKingInCheck(boardViewModel.getActivePawn()->owner, true, boardPosition);
    board->setPawnMoveCheckWarning(isKingInCheck);
    if (isKingInCheck) {
        return;
    }

    // check if field was taken by opposite player and remove it from the board
    if (boardViewModel.didRemoveEnemyOnBoardPosition(boardPosition)) {
        board->removePawnAtBoardPosition(boardPosition);
    }

    // move active pawn to new position
    moveActivePawnToSelectedPoint(point);

    // check if pawn can be promoted
    if (boardViewModel.didPromoteActivePawn()) {
        board->promotePawnAtBoardPosition(boardPosition);
    }

    // check for opposite player king's check
    switch (boardViewModel.getActivePawn()->owner) {
    case PlayerType::black:
        setCheckStateOnPlayerView(PlayerType::white, boardViewModel.isKingInCheck(PlayerType::white, false, boardPosition));
        break;
    case PlayerType::white:
        setCheckStateOnPlayerView(PlayerType::black, boardViewModel.isKingInCheck(PlayerType::black, false, boardPosition));
        break;
    }

    // update active player check state
    setCheckStateOnPlayerView(boardViewModel.getActivePawn()->owner, isKingInCheck);

    // check if game is over
    if (boardViewModel.getWinner()) {
        showCongratulationsScreen(*boardViewModel.getWinner());
        return;
    }

    // change round owner to opposite player
    boardViewModel.discardActivePawn();
    boardViewModel.switchRound();
    blackPlayerView->setActive(boardViewModel.getWhosTurn() == PlayerType::black);
    whitePlayerView->setActive(boardViewModel.getWhosTurn() == PlayerType::white);
}

void GameView::setCheckStateOnPlayerView(PlayerType player, bool isInCheck) {
    switch (player) {
    case PlayerType::black:
        blackPlayerView->setIsInCheck(isInCheck);
        break;
    case PlayerType::white:
        whitePlayerView->setIsInCheck(isInCheck);
        break;
    }
}

// update pawn field position and pawn model position
void GameView::moveActivePawnToSelectedPoint(QPoint point) {
    BoardPosition boardPosition = boardViewModel.getBoardPositionForMousePosition(point);
    board->placeActivePawnAtBoardPosition(boardViewModel.getActivePawn(), boardPosition);
    boardViewModel.setNewPositionForActivePawn(boardPosition);
}

void GameView::releaseActivePawn() {
    if (boardViewModel.getActivePawn() == nullptr) {
        return;
    }

    BasePawnModel *activePawn = boardViewModel.getActivePawn();
    board->placeActivePawnAtBoardPosition(activePawn, activePawn->position);
    board->setPawnMoveCheckWarning(false);
    boardViewModel.discardActivePawn();
}

void GameView::showCongratulationsScreen(PlayerType winner) {
    gameStarted = false;

    scene->clear();

    CongratulationsView *congratulationsView = new CongratulationsView(winner);
    congratulationsView->setRect(0, 0, viewWidth, viewHeight);
}
