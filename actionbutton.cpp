#include "actionbutton.h"
#include <QBrush>
#include <QPen>
#include <QFont>
#include "constants.h"
#include "utils.h"

ActionButton::ActionButton(QString title, QGraphicsItem *parent)
    : QGraphicsRectItem(parent)
{
    setRect(0, 0, 200, 50);

    // 1. 짙은 나무색 배경 설정
    QBrush brush;
    brush.setStyle(Qt::SolidPattern);
    brush.setColor(QColor("#5D4037"));
    setBrush(brush);

    // 2. 고풍스러운 금색 테두리 (Double Line 효과 느낌)
    QPen pen(QColor("#D4AF37"));
    pen.setWidth(3);
    setPen(pen);

    // 3. 클래식 폰트 적용 (Times New Roman 느낌)
    text = new QGraphicsTextItem(title, this);
    QFont font("Georgia", 14, QFont::Bold);
    text->setFont(font);
    text->setDefaultTextColor(QColor("#D4AF37"));

    // 4. 중앙 정렬
    double xPosition = rect().width()/2 - text->boundingRect().width()/2;
    double yPosition = rect().height()/2 - text->boundingRect().height()/2;
    text->setPos(xPosition, yPosition);

    setAcceptHoverEvents(true);
}

void ActionButton::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    emit buttonPressed();
}

void ActionButton::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    // 마우스를 올리면 밝은 나무색으로 변하며 글자가 흰색으로 빛남
    setBrush(QBrush(QColor("#8D6E63")));
    text->setDefaultTextColor(Qt::white);
}

void ActionButton::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    // 원래의 고풍스러운 나무/금색 조합으로 복구
    setBrush(QBrush(QColor("#5D4037")));
    text->setDefaultTextColor(QColor("#D4AF37"));
}

