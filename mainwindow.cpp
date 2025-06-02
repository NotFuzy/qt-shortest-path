#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dijkstra.h"
#include <QMessageBox>
#include <QIntValidator>
#include <QGraphicsTextItem>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    scene = new QGraphicsScene(this);
    ui->graphView->setScene(scene);

    // Валидация ввода для всех QLineEdit
    auto validator = new QIntValidator(1, 9999, this);
    ui->verticesCountLineEdit->setValidator(validator);
    // для ребер меняем с QLineEdit на QComboBox, поэтому валидация для fromEdgeLineEdit и toEdgeLineEdit убирается
    ui->weightLineEdit->setValidator(validator);

    connect(ui->findPathButton, &QPushButton::clicked,
            this, &MainWindow::on_calculateButton_clicked);
}

MainWindow::~MainWindow()
{
    delete graph;
    delete ui;
}

void MainWindow::on_setVerticesCountButton_clicked()
{
    bool ok;
    int count = ui->verticesCountLineEdit->text().toInt(&ok);
    if (!ok || count <= 0) {
        QMessageBox::warning(this, "Ошибка", "Введите корректное количество вершин.");
        return;
    }

    verticesCount = count;
    if (graph) delete graph;
    graph = new Graph(verticesCount);

    // Обновление списков выбора начальной и конечной вершины
    ui->startComboBox->clear();
    ui->endComboBox->clear();
    ui->fromEdgeComboBox->clear();
    ui->toEdgeComboBox->clear();

    for (int i = 1; i <= verticesCount; ++i) {
        QString vertexStr = QString::number(i);
        ui->startComboBox->addItem(vertexStr);
        ui->endComboBox->addItem(vertexStr);
        ui->fromEdgeComboBox->addItem(vertexStr);
        ui->toEdgeComboBox->addItem(vertexStr);
    }

    ui->resultTextEdit->clear();
    ui->resultTextEdit->append(QString("Создан граф с %1 вершинами").arg(verticesCount));
}

void MainWindow::on_addEdgeButton_clicked()
{
    if (!graph) {
        QMessageBox::warning(this, "Ошибка", "Сначала создайте граф.");
        return;
    }

    int from = ui->fromEdgeComboBox->currentText().toInt();
    int to = ui->toEdgeComboBox->currentText().toInt();

    bool ok;
    int weight = ui->weightLineEdit->text().toInt(&ok);

    if (!ok || from <= 0 || to <= 0 || weight <= 0
        || from > verticesCount || to > verticesCount) {
        QMessageBox::warning(this, "Ошибка", "Введите корректные значения.");
        return;
    }

    graph->addEdge(from - 1, to - 1, weight);
    ui->resultTextEdit->append(QString("Добавлено ребро: %1 -> %2 (вес %3)").arg(from).arg(to).arg(weight));
    drawGraph();
}

void MainWindow::on_calculateButton_clicked()
{
    if (!graph) {
        QMessageBox::warning(this, "Ошибка", "Сначала создайте граф.");
        return;
    }

    int from = ui->startComboBox->currentIndex();
    int to = ui->endComboBox->currentIndex();

    if (from == -1 || to == -1 || from >= verticesCount || to >= verticesCount) {
        QMessageBox::warning(this, "Ошибка", "Выберите корректные вершины.");
        return;
    }
    drawGraph();


    DijkstraResult result = DijkstraAlgorithm::findShortestPaths(*graph, from);

    if (!DijkstraAlgorithm::hasPath(result, to)) {
        ui->resultTextEdit->append("Путь не существует.");
        return;
    }

    QString pathStr = QString::fromStdString(DijkstraAlgorithm::getPathString(result, to));
    int dist = result.distances[to];

    ui->resultTextEdit->append(QString("Кратчайший путь: %1").arg(pathStr));
    ui->resultTextEdit->append(QString("Длина пути: %1").arg(dist));
}

void MainWindow::on_clearGraphButton_clicked()
{
    if (graph) {
        delete graph;
        graph = nullptr;
    }
    verticesCount = 0;

    ui->startComboBox->clear();
    ui->endComboBox->clear();
    ui->fromEdgeComboBox->clear();
    ui->toEdgeComboBox->clear();
    ui->resultTextEdit->clear();
    ui->resultTextEdit->append("Граф очищен.");
    scene->clear();

}

void MainWindow::drawGraph()
{
    if (!graph || verticesCount == 0)
        return;

    scene->clear();

    const int radius = 20;
    const int centerX = 250;
    const int centerY = 250;
    const int graphRadius = 200;

    QVector<QPointF> nodePositions;

    // Располагаем вершины по окружности
    for (int i = 0; i < verticesCount; ++i) {
        double angle = (2 * M_PI * i) / verticesCount;
        int x = centerX + graphRadius * cos(angle);
        int y = centerY + graphRadius * sin(angle);
        nodePositions.append(QPointF(x, y));
    }

    // Рисуем рёбра и подписи к ним (веса)
    for (int from = 0; from < verticesCount; ++from) {
        for (const auto& edge : graph->getEdges(from)) {
            int to = edge.first;
            int weight = edge.second;

            scene->addLine(QLineF(nodePositions[from], nodePositions[to]));

            QPointF midPoint = (nodePositions[from] + nodePositions[to]) / 2;
            QGraphicsTextItem* weightText = scene->addText(QString::number(weight));
            QRectF textRect = weightText->boundingRect();
            weightText->setPos(midPoint.x() - textRect.width() / 2, midPoint.y() - textRect.height() / 2);
        }
    }

    // Рисуем вершины и их номера
    for (int i = 0; i < verticesCount; ++i) {
        QPointF pos = nodePositions[i];

        scene->addEllipse(pos.x() - radius, pos.y() - radius,
                          2 * radius, 2 * radius,
                          QPen(), QBrush(Qt::yellow));

        QGraphicsTextItem* label = scene->addText(QString::number(i + 1));
        QRectF labelRect = label->boundingRect();
        label->setPos(pos.x() - labelRect.width() / 2, pos.y() - labelRect.height() / 2);
    }
}

