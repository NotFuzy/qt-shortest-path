#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dijkstra.h"

#include <QMessageBox>
#include <QIntValidator>
#include <QGraphicsTextItem>
#include <QSet>
#include <QPolygonF>
#include <QInputDialog>
#include <cmath>

using namespace graphlib;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), scene(nullptr)
{
    ui->setupUi(this);

    scene = new ClickableScene(this);
    ui->graphView->setScene(scene);

    auto validator = new QIntValidator(1, 9999, this);
    ui->verticesCountLineEdit->setValidator(validator);
    ui->weightLineEdit->setValidator(validator);

    connect(ui->setVerticesCountButton, &QPushButton::clicked, this, &MainWindow::onCreateGraphClicked);
    connect(ui->addEdgeButton, &QPushButton::clicked, this, &MainWindow::onAddEdgeClicked);
    connect(ui->findPathButton, &QPushButton::clicked, this, &MainWindow::onFindPathClicked);
    connect(ui->clearGraphButton, &QPushButton::clicked, this, &MainWindow::onClearGraphClicked);
    connect(ui->directedCheckBox, &QCheckBox::checkStateChanged, this, &MainWindow::onDirectedToggled);

    connect(scene, &ClickableScene::sceneClicked, this, &MainWindow::onSceneClicked);
    connect(ui->designMode, &QCheckBox::toggled, this, &MainWindow::onDesignModeToggled);
    connect(ui->clearHistoryButton, &QPushButton::clicked, this, &MainWindow::onClearHistoryClicked);

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

    QVector<int> path = DijkstraAlgorithm::getPath(result, to);
    QString pathStr = DijkstraAlgorithm::getPathString(result, to);
    int dist = result.distances[to];

    ui->resultTextEdit->append("Кратчайший путь: " + pathStr);
    ui->resultTextEdit->append(QString("Длина пути: %1").arg(dist));

    drawGraph();
    highlightPath(path);
}


void MainWindow::onClearGraphClicked()
{
    if (graph)
        graph->clearGraph();
    graph.reset(); // Чтобы сбросить счётчик и очистить объект

    nodePositions.clear();
    selectedVertexForEdge = -1;

    ui->startComboBox->clear();
    ui->endComboBox->clear();
    ui->fromEdgeComboBox->clear();
    ui->toEdgeComboBox->clear();

    resetUI();
    ui->resultTextEdit->append("Граф очищен.");
    drawGraph(); // Обновляем визуализацию (будет пустая)
}

void MainWindow::onClearHistoryClicked()
{
    ui->resultTextEdit->clear();
    ui->resultTextEdit->append("История очищена.");
}

void MainWindow::resetUI()
{
    ui->startComboBox->clear();
    ui->endComboBox->clear();
    ui->fromEdgeComboBox->clear();
    ui->toEdgeComboBox->clear();
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

    // Автоматически расположить вершины, у которых нет позиции
    while (nodePositions.size() < count) {
        int i = nodePositions.size();
        double angle = 2 * M_PI * i / count;
        double x = center.x() + radius * cos(angle);
        double y = center.y() + radius * sin(angle);
        nodePositions.append(QPointF(x, y));
    }

    for (int i = 0; i < count; ++i) {
        QPointF pos = nodePositions[i];
        scene->addEllipse(pos.x() - nodeRadius, pos.y() - nodeRadius, 2 * nodeRadius, 2 * nodeRadius,
                          QPen(Qt::black), QBrush(Qt::white));

        QGraphicsTextItem* label = scene->addText(QString::number(i + 1));
        label->setPos(pos.x() - 7, pos.y() - 15);
    }
}


void MainWindow::drawEdges()
{
    QSet<QString> drawnEdges;
    int count = graph->verticesCount();
    const int nodeRadius = 20;

    edgeItems.clear(); // Очищаем старые указатели

    for (int from = 0; from < count; ++from) {
        for (const auto& [to, weight] : graph->getEdges(from)) {
            if (!graph->isDirected() && drawnEdges.contains(QString("%1-%2").arg(to).arg(from)))
                continue;

            QPointF p1 = nodePositions[from];
            QPointF p2 = nodePositions[to];
            QLineF line(p1, p2);

            line.setLength(line.length() - nodeRadius);
            line.setP1(line.pointAt(static_cast<qreal>(nodeRadius) / line.length()));

            // Рисуем линию и сохраняем указатель
            QPen edgePen(Qt::black, 2);
            QGraphicsLineItem* lineItem = scene->addLine(line, edgePen);
            edgeItems[qMakePair(from, to)] = lineItem;

            if (!graph->isDirected())
                edgeItems[qMakePair(to, from)] = lineItem;

            // Подпись веса
            QGraphicsTextItem* weightLabel = scene->addText(QString::number(weight));
            weightLabel->setDefaultTextColor(Qt::black);
            weightLabel->setPos((line.p1() + line.p2()) / 2);

            // Рисуем стрелку, если ориентированный
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

void MainWindow::highlightPath(const QVector<int>& path)
{
    // Сброс стиля всех рёбер
    for (auto* item : edgeItems.values()) {
        item->setPen(QPen(Qt::black, 2));
    }

    // Подсветка пути
    for (int i = 0; i < path.size() - 1; ++i) {
        int from = path[i];
        int to = path[i + 1];
        QPair<int, int> key = qMakePair(from, to);
        if (edgeItems.contains(key)) {
            edgeItems[key]->setPen(QPen(Qt::red, 3));
        }
    }
}

void MainWindow::onSceneClicked(const QPointF& pos)
{
    if (!ui->designMode->isChecked()) {
        QMessageBox::warning(this, "Недоступно", "Добавление вершин вручную доступно только в режиме дизайна.");
        return;
    }

    if (!graph) {
        graph = std::make_unique<Graph>(0, ui->directedCheckBox->isChecked());
        nodePositions.clear();
        selectedVertexForEdge = -1;
        // Также очистите комбобоксы, если нужно
        ui->startComboBox->clear();
        ui->endComboBox->clear();
        ui->fromEdgeComboBox->clear();
        ui->toEdgeComboBox->clear();
    }

    // Проверяем, кликнули ли на уже существующую вершину
    int clickedVertex = -1;
    const int clickRadius = 25;
    for (int i = 0; i < nodePositions.size(); ++i) {
        if (QLineF(pos, nodePositions[i]).length() <= clickRadius) {
            clickedVertex = i;
            break;
        }
    }

    if (clickedVertex == -1) {
        // Кликнули на пустом месте — добавляем вершину
        graph->addVertex();
        nodePositions.append(pos);

        int newIndex = graph->verticesCount() - 1;
        QString vertexStr = QString::number(newIndex + 1);

        ui->startComboBox->addItem(vertexStr);
        ui->endComboBox->addItem(vertexStr);
        ui->fromEdgeComboBox->addItem(vertexStr);
        ui->toEdgeComboBox->addItem(vertexStr);

        ui->resultTextEdit->append(QString("Добавлена вершина %1").arg(newIndex + 1));
        drawGraph();  // Обновляем визуализацию сразу после добавления вершины
    } else {
        // Кликнули на существующей вершине — добавляем ребро
        if (selectedVertexForEdge == -1) {
            selectedVertexForEdge = clickedVertex;
            ui->resultTextEdit->append(QString("Выбрана вершина %1 для соединения").arg(clickedVertex + 1));
        } else {
            if (clickedVertex == selectedVertexForEdge) {
                ui->resultTextEdit->append("Нельзя соединить вершину с самой собой.");
                selectedVertexForEdge = -1;
                return;
            }

            bool ok;
            int weight = QInputDialog::getInt(this, "Вес ребра",
                                              QString("Введите вес ребра от %1 до %2:").arg(selectedVertexForEdge + 1).arg(clickedVertex + 1),
                                              1, 1, 9999, 1, &ok);
            if (!ok) {
                ui->resultTextEdit->append("Добавление ребра отменено.");
                selectedVertexForEdge = -1;
                return;
            }

            bool directed = graph->isDirected();
            graph->addEdge(selectedVertexForEdge, clickedVertex, weight);

            QString log = directed
                              ? QString("Добавлено ребро (дизайн): %1 → %2 (вес %3)")
                                    .arg(selectedVertexForEdge + 1).arg(clickedVertex + 1).arg(weight)
                              : QString("Добавлено неориентированное ребро (дизайн): %1 — %2 (вес %3)")
                                    .arg(selectedVertexForEdge + 1).arg(clickedVertex + 1).arg(weight);

            ui->resultTextEdit->append(log);

            selectedVertexForEdge = -1;
            drawGraph();
        }
    }
}


void MainWindow::onDesignModeToggled(bool checked)
{
    ui->groupBoxVertices->setEnabled(!checked);
    ui->groupBoxEdges->setEnabled(!checked);

    if (checked) {
        ui->resultTextEdit->append("Режим дизайна включён: добавление вершин вручную по клику.");
    } else {
        ui->resultTextEdit->append("Режим дизайна отключён: добавление через кнопку.");
    }
}

