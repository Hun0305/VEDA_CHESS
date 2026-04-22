#include "gameview.h"
#include <QGraphicsTextItem>
#include <QColor>
#include <QBrush>
#include "actionbutton.h"
#include "congratulationsview.h"
#include "constants.h"
#include "utils.h"
#include <QInputDialog> // 상단에 추가
#include "rankingdialog.h"
#include <QSqlQuery>
#include <QSqlError>

int viewWidth = 1200;
int viewHeight= 768;

GameView::GameView() {
    gameStarted = false;
    networkManager = nullptr; // 추가

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
    if (scene) {
        scene->clear(); // 여기서 죽는다면, 이전 아이템에 연결된 시그널이 문제일 수 있습니다.
    }

    // --- [추가] 오른쪽 상단 로그인 정보 표시 ---
    if (!loggedInUserId.isEmpty()) {
        QGraphicsTextItem *userInfo = Utils::createTextItem("닉네임: " + loggedInUserId, 15, Qt::yellow);
        // 오른쪽 구석 좌표 계산 (여백 20)
        double infoX = this->width() - userInfo->boundingRect().width() - 20;
        double infoY = 20;
        userInfo->setPos(infoX, infoY);
        scene->addItem(userInfo);
    }

    // 1. 제목 그리기
    QGraphicsTextItem *title = Utils::createTextItem("Lobby", 40, Qt::white);
    double xPosition = this->width()/2 - title->boundingRect().width()/2;
    title->setPos(xPosition, 50);
    scene->addItem(title);

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
        // 수정된 코드
        connect(joinBtn, &ActionButton::buttonPressed, this, [this](){
            myColor = PlayerType::black;

            // 1. 인스턴스 생성 (누락되었던 부분)
            if (!networkManager) {
                networkManager = new NetworkManager(this);
            }

            // 2. 시그널 연결을 먼저 수행하는 것이 안전합니다.
            connect(networkManager, &NetworkManager::dataReceived, this, &GameView::onDataReceived);
            connect(networkManager, &NetworkManager::connected, this, &GameView::startGame);

            // 3. 서버 접속 시도
            networkManager->connectToHost("127.0.0.1", 12345);
        });
        scene->addItem(joinBtn);

        ActionButton *globalJoinBtn = new ActionButton("Global Join");
        globalJoinBtn->setPos(this->width()/2 + 20, 550);
        connect(globalJoinBtn, SIGNAL(buttonPressed()), this, SLOT(globalJoinGame()));
        scene->addItem(globalJoinBtn);

        ActionButton *globalHostBtn = new ActionButton("Global Host");
        globalHostBtn->setPos(globalJoinBtn->boundingRect().width() + 180, 550);
        connect(globalHostBtn, SIGNAL(buttonPressed()), this, SLOT(globalHostGame()));
        scene->addItem(globalHostBtn);
    }

    ActionButton *rankingBtn = new ActionButton("Ranking");

    // 위치 설정: "Back to Menu" 버튼의 왼쪽 혹은 적절한 빈 공간
    // 여기서는 화면 하단 중앙 근처로 배치하겠습니다.
    rankingBtn->setPos(this->width()/2 - rankingBtn->boundingRect().width() / 2, 480);

    // 버튼 클릭 시 RankingDialog 실행 연결 (람다식 활용)
    connect(rankingBtn, &ActionButton::buttonPressed, this, [this]() {
        RankingDialog *dialog = new RankingDialog(this);
        // 창이 닫히면 메모리에서 자동으로 삭제되도록 설정
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->show();
    });
    scene->addItem(rankingBtn);

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
    myColor = PlayerType::white;

    // 1. 기존 매니저와 시그널을 확실히 정리
    if (networkManager) {
        networkManager->disconnect(); // 모든 연결 끊기
        networkManager->deleteLater();
        networkManager = nullptr;
    }

    networkManager = new NetworkManager(this);

    // 2. 연결을 한 번만 수행
    connect(networkManager, &NetworkManager::connected, this, &GameView::startGame, Qt::UniqueConnection);
    connect(networkManager, &NetworkManager::dataReceived, this, &GameView::onDataReceived, Qt::UniqueConnection);

    networkManager->startHosting();
}

void GameView::startGame() {
    gameStarted = false;

    if (scene) {
        scene->clear();
    }

    // BoardViewModel 초기화 (내부 포인터들이 null로 잘 초기화되는지 확인 필요)
    boardViewModel = BoardViewModel();

    // UI 객체 생성 순서: Board -> Panel -> User
    drawBoard();
    drawSettingsPanel();
    drawUserPanel();

    // 닉네임 표시 (중복 생성 방지를 위해 drawUserPanel 이후에 배치)
    if (!loggedInUserId.isEmpty()) {
        QGraphicsTextItem *userDisplay = Utils::createTextItem("닉네임 : " + loggedInUserId, 16, QColor("#FFD700"));
        userDisplay->setPos(this->width() - userDisplay->boundingRect().width() - 20, 20);
        scene->addItem(userDisplay);
    }

    drawTitle(Constants::defaultMargin, 40);
    gameStarted = true;
}

void GameView::quitGame() {
    close();
}

void GameView::drawBoard() {
    if (!scene) return;

    board = new BoardView();
    scene->addItem(board);
    board->draw();
    board->initializePawnFields(boardViewModel.getBlackPawns());
    board->initializePawnFields(boardViewModel.getWhitePawns());
}

void GameView::drawSettingsPanel() {
    // create resign button
    ActionButton *resignButton = new ActionButton("Resign"); // 이름 변경
    double resignXPosition = 680;
    double resignYPosition = 620;
    resignButton->setPos(resignXPosition, resignYPosition);

    connect(resignButton, SIGNAL(buttonPressed()), this, SLOT(resignGame()));
    scene->addItem(resignButton);

    // create quit button
    ActionButton *quitButton = new ActionButton("Quit Game");
    double quitXPosition = 680 + PlayerView::defaultWidthHeight + 20;
    double quitYPosition = 620;
    quitButton->setPos(quitXPosition, quitYPosition);

    connect(quitButton, SIGNAL(buttonPressed()), this, SLOT(quitGame()));
    scene->addItem(quitButton);
}

void GameView::resignGame() {
    if (!gameStarted) return;

    // 1. 네트워크를 통해 상대방에게 항복 선언 전달 (구분자 ';' 포함)
    if (networkManager) {
        networkManager->sendMove("RESIGN");
    }

    // 2. 항복한 쪽의 반대 색상이 승리자
    PlayerType winner = (myColor == PlayerType::white) ? PlayerType::black : PlayerType::white;

    // 3. 승리 화면 띄우기
    showCongratulationsScreen(winner);
}

void GameView::drawUserPanel() {
    blackPlayerView = drawViewForUser(PlayerType::black);
    whitePlayerView = drawViewForUser(PlayerType::white);

    // 로그 창 생성
    logWindow = new QTextEdit();
    logWindow->setReadOnly(true); // 수정 불가
    logWindow->setFixedSize(425, 300); // 크기 설정
    logWindow->setStyleSheet(
        "background-color: rgba(30, 30, 35, 200);"
        "color: #9D8065;"
        "border: 1px solid #9D8065;"
        "font-family: 'Courier New';"
        "font-size: 14px;"
        );

    // GraphicsScene에 위젯 임베딩
    QGraphicsProxyWidget *proxy = scene->addWidget(logWindow);
    proxy->setPos(680, 275); // PlayerView 아래, 버튼 위 빈 공간에 배치

    blackPlayerView->setActive(true);
}

// 로그 메시지 추가
void GameView::addLog(QString message) {
    if (logWindow) {
        logWindow->append("[" + QTime::currentTime().toString("hh:mm:ss") + "] " + message);
        logWindow->ensureCursorVisible(); // 자동 스크롤
    }
}

// (0,0) -> "A1" 형태로 변환하는 도우미 함수
QString GameView::getChessNotation(BoardPosition pos) {
    char column = 'A' + pos.x;
    int row = pos.y + 1;
    return QString("%1%2").arg(column).arg(row);
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
    playerView->setRect(xPosition, yPosition, PlayerView::defaultWidthHeight, PlayerView::defaultWidthHeight/2);
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

        if (pawn != nullptr) {
            // 1. 클릭한 위치의 좌표를 가져옵니다.
            BoardPosition pos = boardViewModel.getBoardPositionForMousePosition(event->pos());

            // 2. 해당 좌표에 있는 말의 주인이 내 색깔(myColor)과 같은지 확인합니다.
            // boardViewModel의 기능을 활용하여 직접 비교합니다.
            if (boardViewModel.getPawnColorAtPosition(pos) != myColor) {
                return; // 내 말이 아니면 무시
            }
        }
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

    // ==========================================================
    // ===== [핵심 방어막] 허공(빈 칸)을 클릭해서 체스말이 없다면 즉시 종료! =====
    if (pawn == nullptr) {
        board->clearHighlights(); // 허공을 누르면 켜져있던 불도 끕니다
        boardViewModel.discardActivePawn(); // 선택된 말도 취소합니다
        return;
    }
    // ==========================================================

    boardViewModel.setActivePawnForField(pawn);

    // ===== 추가: 말이 선택되었으므로 갈 수 있는 범위 표시 =====
    if(boardViewModel.getActivePawn() != nullptr) {
        board->showValidMoves(boardViewModel.getActivePawn(), &boardViewModel);
    }
}

// 2. 패킷 전송 로직 수정 (문제 2 해결)
void GameView::handleSelectingPointForActivePawnByMouse(QPoint point) {
    if (!gameStarted || boardViewModel.getWhosTurn() != myColor) return;
    if (boardViewModel.getActivePawn() == nullptr) return;

    if (!boardViewModel.validatePawnPalcementForMousePosition(point)) return;
    BoardPosition toPosition = boardViewModel.getBoardPositionForMousePosition(point);
    if (!boardViewModel.validatePawnMove(toPosition)) return;

    // [중요] 이동 전의 원래 위치를 '가장 먼저' 저장해야 합니다!
    BoardPosition fromPosition = boardViewModel.getActivePawn()->position;

    // 킹 체크 로직
    bool isKingInCheck = boardViewModel.isKingInCheck(boardViewModel.getActivePawn()->owner, true, toPosition);
    board->setPawnMoveCheckWarning(isKingInCheck);
    if (isKingInCheck) return;

    // 상대 기물 제거 로직
    if (boardViewModel.didRemoveEnemyOnBoardPosition(toPosition)) {

        if (boardViewModel.isKingAtPosition(toPosition)) {
            if (networkManager) {
                // static_cast<int>를 추가하여 Enum을 숫자로 변환합니다.
                QString gameOverPacket = QString("GAMEOVER|%1").arg(static_cast<int>(myColor));
                networkManager->sendMove(gameOverPacket);
            }
            showCongratulationsScreen(myColor);
            board->removePawnAtBoardPosition(toPosition);
            return;
        }

        board->removePawnAtBoardPosition(toPosition);
    }

    // [수정] 여기서 한 번만 이동을 실행합니다.
    moveActivePawnToSelectedPoint(point);

    // 승급 체크
    if (board != nullptr)
    {
    board->clearHighlights();
    }

    // check if pawn can be promoted
    if (boardViewModel.didPromoteActivePawn()) {
        board->promotePawnAtBoardPosition(toPosition);
    }

    if (networkManager) {
        QString movePacket = QString("MOVE|%1|%2|%3|%4")
        .arg(fromPosition.x).arg(fromPosition.y)
            .arg(toPosition.x).arg(toPosition.y);
        networkManager->sendMove(movePacket);
    } else {
        qDebug() << "Critical Error: networkManager is null on Host!";
    }

    // 이동 성공 시점에 추가
    QString pawnName = "Piece"; // 실제 구현 시 activePawn->type에 따라 "Pawn", "Queen" 등으로 분기 가능
    addLog(QString("<b>Move %1</b> from %2 to %3")
               .arg(pawnName)
               .arg(getChessNotation(fromPosition))
               .arg(getChessNotation(toPosition)));

    // 마무리 로직
    boardViewModel.switchRound();
    boardViewModel.discardActivePawn();

    blackPlayerView->setActive(boardViewModel.getWhosTurn() == PlayerType::black);
    whitePlayerView->setActive(boardViewModel.getWhosTurn() == PlayerType::white);
}

void GameView::onDataReceived(QString data) {
    // 1. 안전 장치: 게임이 시작되지 않았거나 화면 전환 중이면 무시
    if (!gameStarted || board == nullptr || scene == nullptr) {
        return;
    }
    QStringList messages = data.split(";", Qt::SkipEmptyParts);

    for (const QString& singleMove : messages) {

        // --- [RESIGN 처리] ---
        if (singleMove == "RESIGN") {
            addLog("<font color='red'>Opponent Resigned the game.</font>");
            showCongratulationsScreen(myColor);
            return;
        }

        // --- [GAMEOVER 처리] ---
        if (singleMove.startsWith("GAMEOVER")) {
            QStringList parts = singleMove.split("|");
            if (parts.size() >= 2) {
                PlayerType winner = static_cast<PlayerType>(parts[1].toInt());
                addLog("<font color='red'>King has been captured! Game Over.</font>");
                showCongratulationsScreen(winner);
                return;
            }
        }

        // --- [MOVE 처리] ---
        QStringList parts = singleMove.split("|");
        if (parts.size() < 5 || parts[0] != "MOVE") continue;

        int fx = parts[1].toInt();
        int fy = parts[2].toInt();
        int tx = parts[3].toInt();
        int ty = parts[4].toInt();

        BoardPosition from(fx, fy);
        BoardPosition to(tx, ty);

        // 2. 객체 참조 전 최종 유효성 검사
        if (!board) break;

        PawnField* remotePawn = board->getPawnAtBoardPosition(from);
        if (remotePawn) {
            boardViewModel.setActivePawnForField(remotePawn);

            if (boardViewModel.didRemoveEnemyOnBoardPosition(to)) {
                board->removePawnAtBoardPosition(to);
            }

            // activePawn이 유효한지 한 번 더 확인
            if (boardViewModel.getActivePawn()) {
                board->placeActivePawnAtBoardPosition(boardViewModel.getActivePawn(), to);
                boardViewModel.setNewPositionForActivePawn(to);
            }

            // 승리자 체크
            if (boardViewModel.getWinner()) {
                showCongratulationsScreen(*boardViewModel.getWinner());
                return;
            }

            boardViewModel.discardActivePawn();
            boardViewModel.switchRound();

            // UI 갱신 전 포인터 확인
            if (blackPlayerView && whitePlayerView) {
                blackPlayerView->setActive(boardViewModel.getWhosTurn() == PlayerType::black);
                whitePlayerView->setActive(boardViewModel.getWhosTurn() == PlayerType::white);
            }

            addLog(QString("Opponent moved to %1").arg(getChessNotation(to)));
        }
    }
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
    board->clearHighlights();
    board->placeActivePawnAtBoardPosition(activePawn, activePawn->position);
    board->setPawnMoveCheckWarning(false);
    boardViewModel.discardActivePawn();
    boardViewModel.discardActivePawn();
}

void GameView::showCongratulationsScreen(PlayerType winner) {
    gameStarted = false;

    // [중요] 결과창이 뜨기 직전에 DB에 전적을 기록합니다.
    bool amIWinner = (winner == myColor);
    updateDatabaseResult(amIWinner);

    scene->clear();
    // CongratulationsView 생성 (이전 단계에서 수정한 대로 인자 2개 전달)
    CongratulationsView *congratulationsView = new CongratulationsView(winner, myColor);
    congratulationsView->setRect(0, 0, width(), height());
    scene->addItem(congratulationsView); // 씬에 추가하는 것 잊지 마세요!
}

void GameView::globalHostGame() {
    myColor = PlayerType::white;
    networkManager = new NetworkManager(this);

    if (networkManager->startHosting(12345)) {
        qDebug() << "Global Server started! Waiting on Port 12345...";

        // TODO: UI에 "접속 대기 중... 내 공인 IP를 친구에게 알려주세요" 메시지 표시

        connect(networkManager, &NetworkManager::connected, this, [this](){
            startGame();
        });
        connect(networkManager, &NetworkManager::dataReceived, this, &GameView::onDataReceived);
    }
}

void GameView::globalJoinGame() {
    // 1. 사용자에게 IP 주소를 입력받는 팝업 창 띄우기
    bool ok;
    QString ipAddress = QInputDialog::getText(nullptr, "Join Global Game",
                                              "Enter Host's IP Address:",
                                              QLineEdit::Normal,
                                              "127.0.0.1", &ok);

    // 2. 사용자가 확인(OK)을 눌렀고 IP를 입력했다면 접속 시도
    if (ok && !ipAddress.isEmpty()) {
        myColor = PlayerType::black;
        networkManager = new NetworkManager(this);

        // 입력받은 IP로 접속 시도
        networkManager->connectToHost(ipAddress, 12345);

        connect(networkManager, &NetworkManager::dataReceived, this, &GameView::onDataReceived);
        connect(networkManager, &NetworkManager::connected, this, &GameView::startGame);
    }
}

void GameView::updateDatabaseResult(bool isWinner) {
    // 1단계에서 전달받은 ID가 있는지 확인
    if (loggedInUserId.isEmpty()) {
        qDebug() << "로그인 정보가 없어 전적을 업데이트할 수 없습니다.";
        return;
    }

    QSqlQuery query;
    if (isWinner) {
        query.prepare("UPDATE users SET wins = wins + 1 WHERE id = :id");
    } else {
        query.prepare("UPDATE users SET losses = losses + 1 WHERE id = :id");
    }
    query.bindValue(":id", loggedInUserId);

    if (!query.exec()) {
        qDebug() << "전적 업데이트 실패:" << query.lastError().text();
    } else {
        qDebug() << "전적 기록 성공! 유저:" << loggedInUserId << " 결과:" << (isWinner ? "승리" : "패배");
    }
}