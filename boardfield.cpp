#include "boardfield.h"
#include "utils.h"
#include "gameview.h"

int BoardField::defaultWidthHeight = 60;
extern GameView *game;

BoardField::BoardField(QColor backgroundColor, BoardPosition position, QGraphicsItem *parent)
    : QGraphicsRectItem(parent), position(position)
{
    this->originalColor = backgroundColor; // 생성될 때 원래 색상 저장
    Utils::setBackgroundColor(backgroundColor, this);
}

BoardPosition BoardField::getPosition() {
    return position;
}

// ===== 새로 추가되는 하이라이트 기능 =====
void BoardField::setHighlight(bool isHighlighted) {
    if (isHighlighted) {
        // 이동 가능한 칸일 경우 색상 변경 (R:144, G:238, B:144, Alpha(투명도):180)
        QColor highlightColor(144, 238, 144, 180);
        Utils::setBackgroundColor(highlightColor, this);
    } else {
        // 하이라이트 해제 시 원래 색으로 복구
        Utils::setBackgroundColor(originalColor, this);
    }
}