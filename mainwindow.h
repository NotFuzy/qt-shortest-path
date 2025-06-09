#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QPointF>
#include <memory>
#include "graph.h"
#include "clickablescene.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onDirectedToggled(int state);
    void onCreateGraphClicked();
    void onAddEdgeClicked();
    void onFindPathClicked();
    void onClearGraphClicked();
    void onSceneClicked(const QPointF& pos);
    void onDesignModeToggled(bool checked);
    void onClearHistoryClicked();


private:
    void resetUI();
    void drawGraph();
    void drawNodes();
    void drawEdges();

    Ui::MainWindow *ui;
    std::unique_ptr<graphlib::Graph> graph;
    ClickableScene *scene;
    QVector<QPointF> nodePositions;
    QMap<QPair<int, int>, QGraphicsLineItem*> edgeItems;
    void highlightPath(const QVector<int>& path);
    int selectedVertexForEdge = -1;


};

#endif // MAINWINDOW_H
