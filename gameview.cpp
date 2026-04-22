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
#include <QLabel>
#include <QGraphicsProxyWidget>

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
    QColor color = QColor("#4E0707");
    brush.setColor(color);
    scene->setBackgroundBrush(brush);

    gameStarted = false;
}

void GameView::displayMainMenu() {
    scene->clear();

    // ==========================================================
    // --- [복구된 코드] 오른쪽 상단 로그인 정보(닉네임) 표시 ---
    if (!loggedInUserId.isEmpty()) {
        QGraphicsTextItem *userDisplay = Utils::createTextItem("닉네임 : " + loggedInUserId, 16, QColor("#FFD700")); // 노란색(골드) 텍스트
        // 화면 오른쪽 구석 좌표 계산 (우측 여백 20, 상단 여백 20)
        double infoX = this->width() - userDisplay->boundingRect().width() - 20;
        double infoY = 20;
        userDisplay->setPos(infoX, infoY);
        scene->addItem(userDisplay);
    }
    // ==========================================================

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
    // 1. 화면 초기화 및 변수 세팅
    if (scene) {
        scene->clear();
    }
    discoveredRooms.clear();
    nextRoomY = 220; // 데이터가 시작될 Y 좌표

    // ---------------------------------------------------------
    // [중요] 정렬을 위한 컬럼 좌표 설정 (모든 행이 이 좌표를 따릅니다)
    // ---------------------------------------------------------
    int colRoom   = 120; // Room Name 시작 위치
    int colHost   = 400; // Host 닉네임 시작 위치 (새로 추가)
    int colIP     = 680; // IP Address 시작 위치
    int colAction = 950; // Action(Join 버튼) 시작 위치
    // ---------------------------------------------------------

    // 2. 우측 상단 닉네임 표시 (금박 느낌의 Gold 색상)
    if (!loggedInUserId.isEmpty()) {
        QGraphicsTextItem *userInfo = Utils::createTextItem("접속자: " + loggedInUserId, 15, QColor("#D4AF37"));
        userInfo->setPos(this->width() - userInfo->boundingRect().width() - 30, 25);
        scene->addItem(userInfo);
    }

    // 3. 로비 제목 (Lobby - 고풍스러운 대형 서체)
    QGraphicsTextItem *title = Utils::createTextItem("Royal Lobby", 45, QColor("#D4AF37"));
    title->setPos(this->width()/2 - title->boundingRect().width()/2, 40);
    scene->addItem(title);

    // 4. 테이블 헤더 설정 (중세풍 금색 서체)
    // 각 제목이 고정된 컬럼 좌표에 딱 맞게 배치됩니다.
    QGraphicsTextItem *roomHeader = Utils::createTextItem("Room Name", 20, QColor("#D4AF37"));
    roomHeader->setPos(colRoom, 150);
    scene->addItem(roomHeader);

    QGraphicsTextItem *hostHeader = Utils::createTextItem("Host", 20, QColor("#D4AF37"));
    hostHeader->setPos(colHost, 150);
    scene->addItem(hostHeader);

    QGraphicsTextItem *ipHeader = Utils::createTextItem("IP Address", 20, QColor("#D4AF37"));
    ipHeader->setPos(colIP, 150);
    scene->addItem(ipHeader);

    QGraphicsTextItem *actionHeader = Utils::createTextItem("Action", 20, QColor("#D4AF37"));
    actionHeader->setPos(colAction, 150);
    scene->addItem(actionHeader);


    // 5. 하단 컨트롤 버튼 생성 (나무/금색 디자인 적용된 ActionButton)
    ActionButton *hostButton = new ActionButton("Host Game");
    ActionButton *refreshBtn = new ActionButton("Refresh");
    ActionButton *rankingBtn = new ActionButton("Ranking");
    ActionButton *backButton = new ActionButton("Back to Menu");

    // 6. 버튼 시그널 연결
    connect(hostButton, SIGNAL(buttonPressed()), this, SLOT(showHostGameSettings()));
    connect(refreshBtn, &ActionButton::buttonPressed, this, &GameView::displayRoomList);
    connect(rankingBtn, &ActionButton::buttonPressed, this, [this]() {
        RankingDialog *dialog = new RankingDialog(this);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->show();
    });
    connect(backButton, SIGNAL(buttonPressed()), this, SLOT(displayMainMenu()));

    // 7. 하단 버튼 일렬 가로 정렬 및 중앙 배치
    QList<ActionButton*> buttons;
    buttons << hostButton << refreshBtn << rankingBtn << backButton;

    double spacing = 30; // 중세풍의 여유로운 간격
    double totalButtonsWidth = 0;

    for(ActionButton* btn : buttons) {
        totalButtonsWidth += btn->boundingRect().width();
    }
    totalButtonsWidth += spacing * (buttons.size() - 1);

    double startX = (this->width() - totalButtonsWidth) / 2;
    double buttonsY = 620; // 하단 배치

    for(ActionButton* btn : buttons) {
        btn->setPos(startX, buttonsY);
        scene->addItem(btn);
        startX += btn->boundingRect().width() + spacing;
    }

    // 8. 네트워크 리스닝 설정
    if (!networkManager) networkManager = new NetworkManager(this);
    disconnect(networkManager, &NetworkManager::gameDiscovered, this, &GameView::onGameDiscovered);
    connect(networkManager, &NetworkManager::gameDiscovered, this, &GameView::onGameDiscovered);
    networkManager->startListeningForGames();
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

    // [수정된 부분] 적을 지우기 전에 해당 위치의 말이 킹인지 먼저 기록합니다!
    bool isEnemyKing = boardViewModel.isKingAtPosition(toPosition);

    // 상대 기물 제거 로직
    if (boardViewModel.didRemoveEnemyOnBoardPosition(toPosition)) {

        if (isEnemyKing) { // 미리 확인해둔 변수를 사용!
            if (networkManager) {
                QString gameOverPacket = QString("GAMEOVER|%1").arg(static_cast<int>(myColor));
                networkManager->sendMove(gameOverPacket);
            }
            board->removePawnAtBoardPosition(toPosition);
            showCongratulationsScreen(myColor); // 이제 정상적으로 승리 화면이 뜹니다!
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
    if (!gameStarted) return; // 중복 실행 방지
    gameStarted = false; // 마우스 클릭 이벤트 차단

    // 1. DB 업데이트 수행
    bool amIWinner = (winner == myColor);
    updateDatabaseResult(amIWinner);

    // 2. scene->clear() 삭제!!! (이것이 튕김의 주범이었습니다)

    // 3. 기존 화면 위에 반투명 결과창 띄우기 (this를 넘겨줍니다)
    CongratulationsView *conView = new CongratulationsView(winner, myColor, this);
    conView->setRect(0, 0, width(), height());
    conView->setZValue(100); // 체스판보다 무조건 맨 위에 오도록 설정
    scene->addItem(conView);
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

#include <QThread> // 파일 상단에 추가

void GameView::updateDatabaseResult(bool isWinner) {
    if (loggedInUserId.isEmpty()) return;

    QSqlQuery query;
    if (isWinner) {
        query.prepare("UPDATE users SET wins = wins + 1 WHERE id = :id");
    } else {
        query.prepare("UPDATE users SET losses = losses + 1 WHERE id = :id");
    }
    query.bindValue(":id", loggedInUserId);

    // 동시 쓰기(Lock) 충돌 방지를 위한 재시도 로직
    bool success = false;
    for (int i = 0; i < 10; ++i) {
        if (query.exec()) {
            success = true;
            qDebug() << "전적 기록 성공! 유저:" << loggedInUserId << (isWinner ? "승리" : "패배");
            break; // 성공하면 루프 탈출
        }
        QThread::msleep(50); // 실패 시 0.05초 대기 후 다시 시도
    }

    if (!success) {
        qDebug() << "DB 업데이트 최종 실패:" << query.lastError().text();
    }
}
void GameView::showHostGameSettings() {
    scene->clear();
    drawTitle(100, 40);

    int labelX = 400;
    int inputX = 600;
    int startY = 250;
    int spacing = 60;

    // 1. 방 이름 (Room Name)
    QLabel *nameLabel = new QLabel("Room Name:");
    nameLabel->setStyleSheet("color: white; font-size: 20px; background: transparent;");
    QGraphicsProxyWidget *nameLabelProxy = scene->addWidget(nameLabel);
    nameLabelProxy->setPos(labelX, startY);

    roomNameInput = new QLineEdit("My Chess Room");
    roomNameInput->setFixedSize(200, 45);
    roomNameInput->setStyleSheet("background-color: white; color: black; font-size: 16px;");
    QGraphicsProxyWidget *nameInputProxy = scene->addWidget(roomNameInput);
    nameInputProxy->setPos(inputX, startY);

    // 2. 네트워크 모드 (Network Mode)
    QLabel *modeLabel = new QLabel("Network Mode:");
    modeLabel->setStyleSheet("color: white; font-size: 20px; background: transparent;");
    QGraphicsProxyWidget *modeLabelProxy = scene->addWidget(modeLabel);
    modeLabelProxy->setPos(labelX, startY + spacing);

    networkModeInput = new QComboBox();
    networkModeInput->addItems({"Local (LAN)", "Global (Public IP)"});
    networkModeInput->setFixedSize(200, 45);
    networkModeInput->setStyleSheet("background-color: white; color: black; font-size: 16px;");
    QGraphicsProxyWidget *modeInputProxy = scene->addWidget(networkModeInput);
    modeInputProxy->setPos(inputX, startY + spacing);

    modeInputProxy->setZValue(10);

    // 3. 포트 번호 (Port)
    QLabel *portLabel = new QLabel("Port:");
    portLabel->setStyleSheet("color: white; font-size: 20px; background: transparent;");
    QGraphicsProxyWidget *portLabelProxy = scene->addWidget(portLabel);
    portLabelProxy->setPos(labelX, startY + spacing * 2);

    portInput = new QLineEdit("12345");
    portInput->setFixedSize(200, 45);
    portInput->setStyleSheet("background-color: white; color: black; font-size: 16px;");
    QGraphicsProxyWidget *portInputProxy = scene->addWidget(portInput);
    portInputProxy->setPos(inputX, startY + spacing * 2);

    // 4. 동작 버튼
    ActionButton *startBtn = new ActionButton("Start Hosting");
    startBtn->setPos(labelX, 500);
    connect(startBtn, SIGNAL(buttonPressed()), this, SLOT(confirmHostGame()));
    scene->addItem(startBtn);

    ActionButton *cancelBtn = new ActionButton("Cancel");
    cancelBtn->setPos(inputX, 500);
    connect(cancelBtn, SIGNAL(buttonPressed()), this, SLOT(displayRoomList()));
    scene->addItem(cancelBtn);
}

void GameView::confirmHostGame() {
    QString roomName = roomNameInput->text();
    QString mode = networkModeInput->currentText();
    int port = portInput->text().toInt();

    if (port <= 0 || port > 65535) port = 12345;

    myColor = PlayerType::white;

    // [핵심 1] 기존 매니저 정리 및 새 매니저 생성
    if (networkManager) {
        networkManager->stopListeningForGames();
        disconnect(networkManager, &NetworkManager::gameDiscovered, this, &GameView::onGameDiscovered);
    } else {
        networkManager = new NetworkManager(this);
    }

    if (networkManager->startHosting(port)) {
        // UDP 브로드캐스트 시작 (닉네임 포함)
        if (mode == "Local (LAN)") {
            networkManager->startBroadcasting(roomName, port, loggedInUserId);
        }

        scene->clear();
        drawTitle(150, 40);

        // 대기 화면 디자인 (양피지 색상)
        QString waitMsg = QString("Waiting for opponent...\nRoom: %1\nPort: %2").arg(roomName).arg(port);
        QGraphicsTextItem *waitText = Utils::createTextItem(waitMsg, 24, QColor("#F5F5DC"));
        waitText->setPos(this->width()/2 - waitText->boundingRect().width()/2, 350);
        scene->addItem(waitText);

        ActionButton *cancelBtn = new ActionButton("Cancel Hosting");
        cancelBtn->setPos(this->width()/2 - cancelBtn->boundingRect().width()/2, 500);

        // 취소 버튼 로직
        connect(cancelBtn, &ActionButton::buttonPressed, this, [this](){
            if (networkManager) {
                networkManager->stopBroadcasting();
                networkManager->deleteLater();
                networkManager = nullptr;
            }
            displayRoomList();
        });
        scene->addItem(cancelBtn);

        // ==========================================================
        // 🚀 [여기서부터가 누락되었던 핵심 코드입니다!]
        // 상대방이 접속(connected)하면 브로드캐스트를 끄고 게임을 시작합니다.
        // ==========================================================
        connect(networkManager, &NetworkManager::connected, this, [this](){
            if (networkManager) {
                networkManager->stopBroadcasting();
            }
            startGame();
        });

        // 데이터 수신 시 처리 함수 연결
        connect(networkManager, &NetworkManager::dataReceived, this, &GameView::onDataReceived);
        // ==========================================================

    } else {
        qDebug() << "Failed to start hosting on port" << port;
    }
}

// [주의] 매개변수에 QString hostName이 추가되었습니다.
void GameView::onGameDiscovered(QString ip, int port, QString roomName, QString hostName) {
    // 1. 중복 검사 (이미 찾은 방은 무시)
    QString roomKey = QString("%1:%2").arg(ip).arg(port);
    if (discoveredRooms.contains(roomKey)) return;
    discoveredRooms.insert(roomKey);

    // ---------------------------------------------------------
    // [핵심] displayRoomList 헤더와 완벽하게 일치시킨 좌표값
    // ---------------------------------------------------------
    int colRoom   = 120;
    int colHost   = 400;
    int colIP     = 680;
    int colAction = 950;
    // ---------------------------------------------------------

    // 2. 방 정보 텍스트 추가 (양피지 느낌의 미색 #F5F5DC 적용)
    // 방 이름
    QGraphicsTextItem *nameItem = Utils::createTextItem(roomName, 16, QColor("#F5F5DC"));
    nameItem->setPos(colRoom, nextRoomY);
    scene->addItem(nameItem);

    // 호스트 닉네임 (새로 추가된 컬럼)
    QGraphicsTextItem *hostItem = Utils::createTextItem(hostName, 16, QColor("#F5F5DC"));
    hostItem->setPos(colHost, nextRoomY);
    scene->addItem(hostItem);

    // IP 주소
    QGraphicsTextItem *ipItem = Utils::createTextItem(ip, 16, QColor("#F5F5DC"));
    ipItem->setPos(colIP, nextRoomY);
    scene->addItem(ipItem);


    // 3. [Join] 버튼 추가 (중세풍 디자인 버튼)
    ActionButton *joinBtn = new ActionButton("Join");
    joinBtn->setScale(0.6); // 리스트 크기에 맞춰 축소

    // 버튼의 Y 좌표를 텍스트 중앙에 맞추기 위해 살짝 보정 (-5)
    joinBtn->setPos(colAction, nextRoomY - 5);

    // 버튼 클릭 시 접속 로직
    connect(joinBtn, &ActionButton::buttonPressed, this, [this, ip, port](){
        myColor = PlayerType::black; // 접속자는 항상 흑색(Black)

        if (networkManager) {
            networkManager->stopListeningForGames(); // 게임 시작 전 재검색 중단

            // 호스트에게 접속 시도
            networkManager->connectToHost(ip, port);

            // 데이터 수신 및 연결 성공 시그널 재연결
            disconnect(networkManager, &NetworkManager::dataReceived, this, &GameView::onDataReceived);
            connect(networkManager, &NetworkManager::dataReceived, this, &GameView::onDataReceived);

            disconnect(networkManager, &NetworkManager::connected, this, &GameView::startGame);
            connect(networkManager, &NetworkManager::connected, this, &GameView::startGame);
        }
    });

    scene->addItem(joinBtn);

    // 4. 다음 방이 표시될 위치 계산 (행 간격 60)
    nextRoomY += 60;
}