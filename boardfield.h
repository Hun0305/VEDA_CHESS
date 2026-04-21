#ifndef BOARDFIELD_H
#define BOARDFIELD_H

#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QColor>
#include "boardposition.h"

class BoardField: public QGraphicsRectItem {

public:
    static int defaultWidthHeight;
    BoardField(QColor backgroundColor, BoardPosition position, QGraphicsItem *parent = nullptr);

    BoardPosition position;
    QColor originalColor; // 원래 타일 색상을 기억할 변수 추가

    BoardPosition getPosition();
    void setHighlight(bool isHighlighted); // 하이라이트 껐다 켜는 함수 추가
};

#endif // BOARDFIELD_H