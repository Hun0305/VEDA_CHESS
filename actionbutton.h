#ifndef ACTIONBUTTON_H
#define ACTIONBUTTON_H

#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QObject>
#include <QGraphicsTextItem>

class ActionButton : public QObject, public QGraphicsRectItem {
    Q_OBJECT
public:
    // [핵심] explicit를 붙여서 모호함(Ambiguous) 에러를 완벽하게 차단합니다.
    explicit ActionButton(QString title, QGraphicsItem *parent = nullptr);

signals:
    void buttonPressed();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    QGraphicsTextItem *text; // 글자색 반전을 위해 꼭 필요합니다!
};

#endif // ACTIONBUTTON_H