#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

#include "graph.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_setVerticesCountButton_clicked();
    void on_addEdgeButton_clicked();
    void on_calculateButton_clicked();
    void on_clearGraphButton_clicked();

private:
    Ui::MainWindow *ui;

    QGraphicsScene* scene = nullptr;
    Graph* graph = nullptr;
    int verticesCount = 0;

    void drawGraph();
};

#endif // MAINWINDOW_H
