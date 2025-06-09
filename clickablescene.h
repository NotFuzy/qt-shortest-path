#ifndef CLICKABLESCENE_H
#define CLICKABLESCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

class ClickableScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit ClickableScene(QObject *parent = nullptr);

signals:
    void sceneClicked(const QPointF& pos);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
};

#endif // CLICKABLESCENE_H
