#include "clickablescene.h"

ClickableScene::ClickableScene(QObject *parent) : QGraphicsScene(parent) {}

void ClickableScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit sceneClicked(event->scenePos());
    }
    QGraphicsScene::mousePressEvent(event);
}
