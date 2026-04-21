#ifndef BOARDVIEWMODEL_H
#define BOARDVIEWMODEL_H

#include <QObject>
#include <QList>
#include <QPoint>
#include "boardposition.h"
#include "basepawnmodel.h"
#include "pawnviewmodel.h"
#include "boardfield.h"

// PawnField와의 상호 참조 에러를 방지하기 위해 전방 선언을 사용합니다.
class PawnField;

class BoardViewModel {
public:
    BoardViewModel();

    // [하이라이트 기능 추가] 모든 칸 리스트 및 제어 함수
    QList<BoardField*> allFields;
    void showPossibleMoves(PawnField* pawn);
    void clearHighlights();
    BoardField* getFieldAt(BoardPosition pos);

    // [기존 게임 로직 함수 유지]
    QList<BasePawnModel*> getBlackPawns();
    QList<BasePawnModel*> getWhitePawns();
    BasePawnModel* getActivePawn();
    PlayerType getWhosTurn();
    PlayerType* getWinner();

    void setActivePawnForField(PawnField *pawn);
    void setNewPositionForActivePawn(BoardPosition position);
    void discardActivePawn();

    BoardPosition getBoardPositionForMousePosition(QPoint position);
    bool validatePawnPalcementForMousePosition(QPoint position);

    // 이동 유효성 검사 (이미 구현된 핵심 로직)
    bool validatePawnMove(BoardPosition positionToMove, BasePawnModel *pawnToValidate = nullptr, BoardPosition *requestedActivePawnPosition = nullptr);
    bool didRemoveEnemyOnBoardPosition(BoardPosition boardPosition);
    bool isKingInCheck(PlayerType owner, bool isCheckingActivePlayer = false, BoardPosition positionToMoveActivePlayer = {-1, -1});
    bool didPromoteActivePawn();
    void switchRound();

private:
    BasePawnModel *activePawn;
    PlayerType whosTurn;
    QList<BasePawnModel*> blackPawns;
    QList<BasePawnModel*> whitePawns;
    PawnViewModel pawnViewModel;
    PlayerType *winner;
    bool isEnPassantAvailable;

    void initializePawns();
    void initializePawnsForRow(int rowNumber, PlayerType owner);
    BasePawnModel* getPawnOnBoardPosition(BoardPosition boardPosition);
    bool validateAnotherPawnIntersection(BoardPosition positionToMove, BasePawnModel *pawnToValidate, BoardPosition *requestedActivePawnPosition = nullptr);
    bool validateKingsCheckForPawns(QList<BasePawnModel*> pawns, bool isCheckingActivePlayer, BasePawnModel *king, BoardPosition positionToMoveActivePlayer);
};

#endif // BOARDVIEWMODEL_H