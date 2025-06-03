#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dijkstra.h"

#include <QMessageBox>
#include <QIntValidator>
#include <QGraphicsTextItem>
#include <QSet>
#include <QPolygonF>
#include <cmath>

using namespace graphlib;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), scene(new QGraphicsScene(this))
{
    ui->setupUi(this);
    ui->graphView->setScene(scene);

    auto validator = new QIntValidator(1, 9999, this);
    ui->verticesCountLineEdit->setValidator(validator);
    ui->weightLineEdit->setValidator(validator);

    connect(ui->setVerticesCountButton, &QPushButton::clicked, this, &MainWindow::onCreateGraphClicked);
    connect(ui->addEdgeButton, &QPushButton::clicked, this, &MainWindow::onAddEdgeClicked);
    connect(ui->findPathButton, &QPushButton::clicked, this, &MainWindow::onFindPathClicked);
    connect(ui->clearGraphButton, &QPushButton::clicked, this, &MainWindow::onClearGraphClicked);
    connect(ui->directedCheckBox, &QCheckBox::checkStateChanged, this, &MainWindow::onDirectedToggled);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onDirectedToggled(int state)
{
    if (graph) {
        graph->setDirected(state == Qt::Checked);
        drawGraph();
    }
}

void MainWindow::onCreateGraphClicked()
{
    bool ok;
    int count = ui->verticesCountLineEdit->text().toInt(&ok);
    if (!ok || count <= 0) {
        QMessageBox::warning(this, "Ошибка", "Введите корректное количество вершин.");
        return;
    }

    graph = std::make_unique<Graph>(count, ui->directedCheckBox->isChecked());

    ui->startComboBox->clear();
    ui->endComboBox->clear();
    ui->fromEdgeComboBox->clear();
    ui->toEdgeComboBox->clear();

    for (int i = 1; i <= count; ++i) {
        QString vertexStr = QString::number(i);
        ui->startComboBox->addItem(vertexStr);
        ui->endComboBox->addItem(vertexStr);
        ui->fromEdgeComboBox->addItem(vertexStr);
        ui->toEdgeComboBox->addItem(vertexStr);
    }

    ui->resultTextEdit->clear();
    ui->resultTextEdit->append(QString("Создан граф с %1 вершинами").arg(count));
    drawGraph();
}

void MainWindow::onAddEdgeClicked()
{
    if (!graph) {
        QMessageBox::warning(this, "Ошибка", "Сначала создайте граф.");
        return;
    }

    int from = ui->fromEdgeComboBox->currentIndex();
    int to = ui->toEdgeComboBox->currentIndex();

    bool ok;
    int weight = ui->weightLineEdit->text().toInt(&ok);

    if (!ok || weight <= 0) {
        QMessageBox::warning(this, "Ошибка", "Введите корректные значения.");
        return;
    }

    graph->addEdge(from, to, weight);

    QString log = graph->isDirected()
                      ? QString("Добавлено ребро: %1 → %2 (вес %3)").arg(from + 1).arg(to + 1).arg(weight)
                      : QString("Добавлено неориентированное ребро: %1 — %2 (вес %3)").arg(from + 1).arg(to + 1).arg(weight);

    ui->resultTextEdit->append(log);
    drawGraph();
}

void MainWindow::onFindPathClicked()
{
    if (!graph) {
        QMessageBox::warning(this, "Ошибка", "Сначала создайте граф.");
        return;
    }

    int from = ui->startComboBox->currentIndex();
    int to = ui->endComboBox->currentIndex();

    if (from == -1 || to == -1) {
        QMessageBox::warning(this, "Ошибка", "Выберите корректные вершины.");
        return;
    }

    DijkstraResult result = DijkstraAlgorithm::findShortestPaths(*graph, from);

    if (!DijkstraAlgorithm::hasPath(result, to)) {
        ui->resultTextEdit->append("Путь не существует.");
        return;
    }

    QString pathStr = DijkstraAlgorithm::getPathString(result, to);
    int dist = result.distances[to];

    ui->resultTextEdit->append("Кратчайший путь: " + pathStr);
    ui->resultTextEdit->append(QString("Длина пути: %1").arg(dist));
    drawGraph();
}

void MainWindow::onClearGraphClicked()
{
    if (graph)
        graph->clearGraph();

    resetUI();
    ui->resultTextEdit->append("Граф очищен.");
}

void MainWindow::resetUI()
{
    ui->startComboBox->clear();
    ui->endComboBox->clear();
    ui->fromEdgeComboBox->clear();
    ui->toEdgeComboBox->clear();
    ui->resultTextEdit->clear();
    scene->clear();
}

void MainWindow::drawGraph()
{
    if (!graph)
        return;

    scene->clear();
    drawNodes();
    drawEdges();
}

void MainWindow::drawNodes()
{
    const int radius = 200;
    const int nodeRadius = 20;
    const QPointF center(300, 300);

    int count = graph->verticesCount();
    nodePositions.resize(count);

    for (int i = 0; i < count; ++i) {
        double angle = 2 * M_PI * i / count;
        double x = center.x() + radius * cos(angle);
        double y = center.y() + radius * sin(angle);
        nodePositions[i] = QPointF(x, y);

        scene->addEllipse(x - nodeRadius, y - nodeRadius, 2 * nodeRadius, 2 * nodeRadius,
                          QPen(Qt::black), QBrush(Qt::white));

        QGraphicsTextItem* label = scene->addText(QString::number(i + 1));
        label->setPos(x - 7, y - 15);
    }
}

void MainWindow::drawEdges()
{
    QSet<QString> drawnEdges;
    int count = graph->verticesCount();
    const int nodeRadius = 20;

    for (int from = 0; from < count; ++from) {
        for (const auto& [to, weight] : graph->getEdges(from)) {
            if (!graph->isDirected() && drawnEdges.contains(QString("%1-%2").arg(to).arg(from)))
                continue;

            QPointF p1 = nodePositions[from];
            QPointF p2 = nodePositions[to];
            QLineF line(p1, p2);

            line.setLength(line.length() - nodeRadius);
            line.setP1(line.pointAt(static_cast<qreal>(nodeRadius) / line.length()));
            scene->addLine(line, QPen(Qt::black));

            // Подпись веса
            QGraphicsTextItem* weightLabel = scene->addText(QString::number(weight));
            weightLabel->setPos((line.p1() + line.p2()) / 2);

            if (graph->isDirected()) {
                double angle = std::atan2(-line.dy(), line.dx());
                QPointF arrowTip = p2 - QPointF(std::cos(angle) * nodeRadius,
                                                -std::sin(angle) * nodeRadius);

                QPointF arrowP1 = arrowTip - QPointF(std::cos(angle + M_PI / 6) * 10,
                                                     -std::sin(angle + M_PI / 6) * 10);
                QPointF arrowP2 = arrowTip - QPointF(std::cos(angle - M_PI / 6) * 10,
                                                     -std::sin(angle - M_PI / 6) * 10);

                QPolygonF arrowHead;
                arrowHead << arrowTip << arrowP1 << arrowP2;
                scene->addPolygon(arrowHead, QPen(Qt::black), QBrush(Qt::black));
            }

            if (!graph->isDirected())
                drawnEdges.insert(QString("%1-%2").arg(from).arg(to));
        }
    }
}
