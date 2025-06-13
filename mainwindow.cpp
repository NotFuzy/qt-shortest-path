#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dijkstra.h"
#include "dijkstrastepper.h"

#include <QMessageBox>
#include <QIntValidator>
#include <QGraphicsTextItem>
#include <QSet>
#include <QVector>
#include <QPolygonF>
#include <QInputDialog>
#include <QFileDialog>
#include <QFile>
#include <QDebug>
#include <cmath>

using namespace graphlib;
using graphlib::DijkstraStepper;

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
    connect(ui->stepButton, &QPushButton::clicked, this, &MainWindow::onStepClicked);

    connect(ui->actionToggleTheme, &QAction::triggered, this, &MainWindow::toggleTheme);

    // Подключаем переключение языка к существующим действиям
    connect(ui->actionEnglish, &QAction::triggered, [this]() {
        switchLanguage("en");
    });

    connect(ui->actionRussia, &QAction::triggered, [this]() {
        switchLanguage("ru");
    });



    applyStyle("light");



}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::switchLanguage(const QString &locale)
{
    qApp->removeTranslator(&translator);

    QString qmFile = QString(":/app_%1.qm").arg(locale);
    if (translator.load(qmFile)) {
        qApp->installTranslator(&translator);
        ui->retranslateUi(this);
    } else {
        qDebug() << "Не удалось загрузить перевод:" << qmFile;
    }
}

void MainWindow::retranslateUi()
{
    ui->retranslateUi(this);
}


void MainWindow::applyStyle(const QString& styleName)
{
    QString resourcePath;
    if (styleName == "dark") {
        resourcePath = ":/dark.qss";
    } else {
        resourcePath = ":/light.qss";
    }

    QFile file(resourcePath);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QString styleSheet = QString::fromUtf8(file.readAll());
        qApp->setStyleSheet(styleSheet);
        file.close();
    } else {
        QMessageBox::warning(nullptr, QObject::tr("Ошибка"), QObject::tr("Не удалось загрузить файл стилей!"));
    }
}

void MainWindow::on_actionToggleTheme_triggered(bool checked)
{
    if (checked) {
        applyStyle("dark");
    } else {
        applyStyle("light");
    }
}

void MainWindow::toggleTheme()
{
    darkTheme = !darkTheme;

    QString stylePath = darkTheme ? ":/dark.qss" : ":/style.qss";
    QFile styleFile(stylePath);

    if (styleFile.open(QFile::ReadOnly)) {
        QString style = QLatin1String(styleFile.readAll());
        qApp->setStyleSheet(style);
    }

    drawGraph(); // Перерисовать граф с новыми цветами
}


void MainWindow::on_action_1_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Сохранить граф"), "", "Graph Files (*.txt)");
    if (!fileName.isEmpty()) {
        saveGraphToFile(fileName);
        ui->resultTextEdit->append(tr("Граф сохранён в файл: %1").arg(fileName));
    }
}

void MainWindow::on_action_2_triggered()
{
    QString filePath = QFileDialog::getSaveFileName(this, tr("Сохранить изображение графа"), "", "PNG (*.png);;JPEG (*.jpg)");
    if (!filePath.isEmpty()) {
        saveSceneAsImage(filePath);
        ui->resultTextEdit->append(tr("Граф сохранён как изображение: %1").arg(filePath));
    }
}

void MainWindow::on_action_3_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Загрузить граф"), "", "Graph Files (*.txt)");
    if (!fileName.isEmpty()) {
        loadGraphFromFile(fileName);
        ui->resultTextEdit->append(tr("Граф загружен из файла: %1").arg(fileName));
    }
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
        QMessageBox::warning(this, tr("Ошибка"), tr("Введите корректное количество вершин."));
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
    ui->resultTextEdit->append(QString(tr("Создан граф с %1 вершинами")).arg(count));
    drawGraph();
}

void MainWindow::onAddEdgeClicked()
{
    if (!graph) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Сначала создайте граф."));
        return;
    }

    int from = ui->fromEdgeComboBox->currentIndex();
    int to = ui->toEdgeComboBox->currentIndex();

    bool ok;
    int weight = ui->weightLineEdit->text().toInt(&ok);

    if (!ok || weight <= 0) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Введите корректные значения."));
        return;
    }

    graph->addEdge(from, to, weight);

    QString log = graph->isDirected()
                      ? QString(tr("Добавлено ребро: %1 → %2 (вес %3)")).arg(from + 1).arg(to + 1).arg(weight)
                      : QString(tr("Добавлено неориентированное ребро: %1 — %2 (вес %3)")).arg(from + 1).arg(to + 1).arg(weight);

    ui->resultTextEdit->append(log);
    drawGraph();
}
    void MainWindow::onFindPathClicked()
{
    if (!graph) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Сначала создайте граф."));
        return;
    }

    int from = ui->startComboBox->currentIndex();
    int to = ui->endComboBox->currentIndex();

    if (from == -1 || to == -1) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Выберите корректные вершины."));
        return;
    }

    DijkstraResult result = DijkstraAlgorithm::findShortestPaths(*graph, from);

    if (!DijkstraAlgorithm::hasPath(result, to)) {
        ui->resultTextEdit->append(tr("Путь не существует."));
        return;
    }

    QVector<int> path = DijkstraAlgorithm::getPath(result, to);
    QString pathStr = DijkstraAlgorithm::getPathString(result, to);
    int dist = result.distances[to];

    ui->resultTextEdit->append(tr("Кратчайший путь: %1").arg(pathStr));
    ui->resultTextEdit->append(tr("Длина пути: %1").arg(dist));


    drawGraph();
    highlightPath(path);
}


void MainWindow::onClearGraphClicked()
{
    if (graph)
        graph->clearGraph();
    graph.reset();

    nodePositions.clear();
    selectedVertexForEdge = -1;

    ui->startComboBox->clear();
    ui->endComboBox->clear();
    ui->fromEdgeComboBox->clear();
    ui->toEdgeComboBox->clear();

    resetUI();
    ui->resultTextEdit->append(tr("Граф очищен."));
    drawGraph();
}

void MainWindow::onClearHistoryClicked()
{
    ui->resultTextEdit->clear();
    ui->resultTextEdit->append(tr("История очищена."));
}

void MainWindow::resetUI()
{
    QList<QGraphicsItem*> items = scene->items();
    for (QGraphicsItem* item : items) {
        scene->removeItem(item);
        delete item;
    }

    ui->startComboBox->clear();
    ui->endComboBox->clear();
    ui->fromEdgeComboBox->clear();
    ui->toEdgeComboBox->clear();

    ui->resultTextEdit->clear();
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

    edgeItems.clear();

    QColor lineColor = darkTheme ? Qt::white : Qt::black;
    QColor textColor = darkTheme ? Qt::white : Qt::black;

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
            QPen edgePen(lineColor, 2);
            QGraphicsLineItem* lineItem = scene->addLine(line, edgePen);
            edgeItems[qMakePair(from, to)] = lineItem;

            if (!graph->isDirected())
                edgeItems[qMakePair(to, from)] = lineItem;

            // Подпись веса
            QGraphicsTextItem* weightLabel = scene->addText(QString::number(weight));
            weightLabel->setDefaultTextColor(textColor);
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

                scene->addPolygon(arrowHead, QPen(lineColor), QBrush(lineColor));
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
        QMessageBox::warning(this, tr("Недоступно"), tr("Добавление вершин вручную доступно только в режиме дизайна."));
        return;
    }

    if (!graph) {
        graph = std::make_unique<Graph>(0, ui->directedCheckBox->isChecked());
        nodePositions.clear();
        selectedVertexForEdge = -1;

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

        ui->resultTextEdit->append(tr("Добавлена вершина %1").arg(newIndex + 1));
        drawGraph();  // Обновляем визуализацию сразу после добавления вершины
    } else {
        // Кликнули на существующей вершине — добавляем ребро
        if (selectedVertexForEdge == -1) {
            selectedVertexForEdge = clickedVertex;
            ui->resultTextEdit->append(tr("Выбрана вершина %1 для соединения").arg(clickedVertex + 1));
        } else {
            if (clickedVertex == selectedVertexForEdge) {
                ui->resultTextEdit->append(tr("Нельзя соединить вершину с самой собой."));
                selectedVertexForEdge = -1;
                return;
            }

            bool ok;
            int weight = QInputDialog::getInt(this, tr("Вес ребра"),
                                              tr("Введите вес ребра от %1 до %2:").arg(selectedVertexForEdge + 1).arg(clickedVertex + 1),
                                              1, 1, 9999, 1, &ok);
            if (!ok) {
                ui->resultTextEdit->append(tr("Добавление ребра отменено."));
                selectedVertexForEdge = -1;
                return;
            }

            bool directed = graph->isDirected();
            graph->addEdge(selectedVertexForEdge, clickedVertex, weight);

            QString log = directed
                              ? tr("Добавлено ребро (дизайн): %1 → %2 (вес %3)").arg(selectedVertexForEdge + 1).arg(clickedVertex + 1).arg(weight)
                              : tr("Добавлено неориентированное ребро (дизайн): %1 — %2 (вес %3)").arg(selectedVertexForEdge + 1).arg(clickedVertex + 1).arg(weight);

            ui->resultTextEdit->append(log);

            selectedVertexForEdge = -1;
            drawGraph();
        }
    }
}

void MainWindow::saveGraphToFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);

    out << "# Тип графа: directed или undirected\n";
    out << (graph->isDirected() ? "directed" : "undirected") << "\n\n";

    out << "# Количество вершин\n";
    out << graph->verticesCount() << "\n\n";

    out << "# Координаты вершин (x y) по одной на строку\n";
    for (const QPointF& pos : nodePositions) {
        out << pos.x() << " " << pos.y() << "\n";
    }
    out << "\n";

    out << "# Рёбра в формате: from to weight\n";
    for (int from = 0; from < graph->verticesCount(); ++from) {
        const auto& edges = graph->getEdges(from);
        for (const auto& edge : edges) {
            int to = edge.first;
            int weight = edge.second;
            out << from << " " << to << " " << weight << "\n";
        }
    }

    file.close();
}



void MainWindow::loadGraphFromFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    resetUI();
    graph.reset();
    nodePositions.clear();

    QTextStream in(&file);
    QString line;

    while (!in.atEnd()) {
        line = in.readLine().trimmed();
        if (!line.startsWith("#") && !line.isEmpty())
            break;
    }
    bool directed = (line == "directed");

    while (!in.atEnd()) {
        line = in.readLine().trimmed();
        if (!line.startsWith("#") && !line.isEmpty())
            break;
    }
    int vertexCount = line.toInt();
    graph = std::make_unique<Graph>(vertexCount, directed);

    while (!in.atEnd()) {
        line = in.readLine().trimmed();
        if (!line.startsWith("#") && !line.isEmpty())
            break;
    }

    int coordLinesRead = 0;
    do {
        if (line.startsWith("#") || line.isEmpty()) break;
        QTextStream coordStream(&line);
        qreal x, y;
        coordStream >> x >> y;
        nodePositions.append(QPointF(x, y));

        QString label = QString::number(coordLinesRead + 1);
        ui->startComboBox->addItem(label);
        ui->endComboBox->addItem(label);
        ui->fromEdgeComboBox->addItem(label);
        ui->toEdgeComboBox->addItem(label);

        coordLinesRead++;
        line = in.readLine().trimmed();
    } while (coordLinesRead < vertexCount && !in.atEnd());

    while (!in.atEnd()) {
        line = in.readLine().trimmed();
        if (line.startsWith("#") || line.isEmpty()) continue;

        QTextStream edgeStream(&line);
        int from, to, weight;
        edgeStream >> from >> to >> weight;
        graph->addEdge(from, to, weight);
    }

    file.close();
    drawGraph();
}


void MainWindow::saveSceneAsImage(const QString& filePath)
{
    if (!scene)
        return;

    QRectF bounds = scene->itemsBoundingRect();
    QImage image(bounds.size().toSize(), QImage::Format_ARGB32);
    image.fill(Qt::white);

    QPainter painter(&image);
    scene->render(&painter, QRectF(), bounds);
    image.save(filePath);
}


void MainWindow::onDesignModeToggled(bool checked)
{
    ui->groupBoxVertices->setEnabled(!checked);
    ui->groupBoxEdges->setEnabled(!checked);

    if (checked) {
        ui->resultTextEdit->append(tr("Режим дизайна включён: добавление вершин вручную по клику."));
    } else {
        ui->resultTextEdit->append(tr("Режим дизайна отключён: добавление через кнопку."));
    }
}

void MainWindow::onStepClicked()
{
    if (!stepper) {
        int start = ui->startComboBox->currentIndex();
        if (start < 0) {
            ui->resultTextEdit->append(tr("Выберите начальную вершину."));
            return;
        }
        stepper = std::make_unique<graphlib::DijkstraStepper>(*graph, start);
        ui->resultTextEdit->append(tr("Начато пошаговое выполнение алгоритма Дейкстры."));
    }

    graphlib::StepChange change;
    do {
        change = stepper->nextStep();
    } while (stepper->hasNextStep() && change.currentVertex == -1);

    // Алгоритм завершён — очередь пуста
    if (!stepper->hasNextStep() && change.currentVertex == -1) {
        ui->resultTextEdit->append(tr("Алгоритм завершён."));

        int end = ui->endComboBox->currentIndex();
        const auto& state = stepper->getState();

        if (end < 0 || end >= state.distances.size()) {
            ui->resultTextEdit->append(tr("Выберите конечную вершину."));
        } else if (state.distances[end] == std::numeric_limits<int>::max()) {
            ui->resultTextEdit->append(tr("Путь не существует."));
        } else {
            QVector<int> path;
            for (int at = end; at != -1; at = state.previous[at])
                path.append(at);
            std::reverse(path.begin(), path.end());

            QStringList pathStrList;
            for (int v : path)
                pathStrList << QString::number(v + 1);

            ui->resultTextEdit->append(QString(tr("Кратчайший путь: %1")).arg(pathStrList.join(" → ")));
            ui->resultTextEdit->append(QString(tr("Длина пути: %1")).arg(state.distances[end]));

            highlightPath(path);
        }

        stepper.reset(); // очистка после завершения
        return;
    }

    // Отображение текущего шага
    int current = stepper->getState().currentVertex + 1;
    if (change.vertexUpdated != -1) {
        ui->resultTextEdit->append(
            QString(tr("Обработана вершина %1: обновлено расстояние до вершины %2 с %3 на %4 (через %5)"))
                .arg(current)
                .arg(change.vertexUpdated + 1)
                .arg(change.oldDistance == std::numeric_limits<int>::max() ? "∞" : QString::number(change.oldDistance))
                .arg(change.newDistance)
                .arg(change.fromVertex + 1)
            );
    } else {
        ui->resultTextEdit->append(
            QString(tr("Обработана вершина %1: соседние вершины не обновлены")).arg(current)
            );
    }

    updateGraphVisualization(change);
}




void MainWindow::updateGraphVisualization(const graphlib::StepChange& change)
{
    drawGraph(); // сбрасываем всё, чтобы очистить старые подсветки

    // Подсветить текущую вершину
    int cur = stepper->getState().currentVertex;
    if (cur >= 0 && cur < nodePositions.size()) {
        QGraphicsEllipseItem* item = scene->addEllipse(
            nodePositions[cur].x() - 15, nodePositions[cur].y() - 15,
            30, 30, QPen(Qt::black), QBrush(Qt::yellow)
            );
        item->setZValue(1); // выше ребер
    }

    // Подсветить обновлённое ребро
    if (change.vertexUpdated != -1) {
        auto key = QPair<int, int>(change.fromVertex, change.vertexUpdated);
        if (edgeItems.contains(key)) {
            edgeItems[key]->setPen(QPen(Qt::red, 3)); // толщина 3, красный цвет
        }
    }
}

