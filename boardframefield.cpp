#include "boardframefield.h"
#include "constants.h"
#include "utils.h"
#include "gameview.h"
#include <QFont>


BoardFrameField::BoardFrameField(QGraphicsItem *parent): QGraphicsRectItem(parent) {
    QColor backgroundColor = QColor(55, 51, 63);
    Utils::setBackgroundColor(backgroundColor, this);
    setPen(Qt::NoPen);
}

void BoardFrameField::setTitle(QString title) {
    QGraphicsTextItem *titleItem = Utils::createTextItem(title, 16, Constants::defaultTextColor);

    double titleXPosition = this->pos().x() + this->boundingRect().width()/2 - titleItem->boundingRect().width()/2;
    double titleYPosition = this->pos().y() + this->boundingRect().height()/2 - titleItem->boundingRect().height()/2;
    titleItem->setPos(titleXPosition, titleYPosition);

    // [수정] game->scene 대신 현재 아이템이 속한 scene에 추가
    if (this->scene()) {
        this->scene()->addItem(titleItem);
    }
}
