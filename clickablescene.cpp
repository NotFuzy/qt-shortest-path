#include "clickablescene.h"
#include <QGraphicsView>

ClickableScene::ClickableScene(QObject *parent) : QGraphicsScene(parent) {}

void ClickableScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit sceneClicked(event->scenePos());
    }
    QGraphicsScene::mousePressEvent(event);
}

void ClickableScene::wheelEvent(QGraphicsSceneWheelEvent *event) {
    if (views().isEmpty()) {
        QGraphicsScene::wheelEvent(event);
        return;
    }
    const double factor = 1.15;
    auto view = views().first();
    if (event->delta() > 0)
        view->scale(factor, factor);
    else
        view->scale(1.0 / factor, 1.0 / factor);
    event->accept();
}
