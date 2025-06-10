#ifndef DIJKSTRASTEPPER_H
#define DIJKSTRASTEPPER_H

#include <QObject>
#include <QSet>
#include <QVector>
#include <QPair>
#include <queue>
#include <limits>
#include "graph.h"  // Твой класс Graph

namespace graphlib {

struct StepChange {
    int vertexUpdated = -1;      // обновленная вершина или -1 если обновлений нет
    int oldDistance = std::numeric_limits<int>::max();
    int newDistance = std::numeric_limits<int>::max();
    int fromVertex = -1;         // вершина, через которую обновилось расстояние

    int currentVertex = -1;      // текущая обрабатываемая вершина

    // Вершины и рёбра для подсветки (например, ребро как пара (откуда, куда))
    QVector<int> highlightVertices;
    QVector<QPair<int,int>> highlightEdges;
};

struct DijkstraStepState {
    QVector<int> distances;
    QVector<int> previous;
    QSet<int> visited;
    int currentVertex = -1;
};

class DijkstraStepper : public QObject
{
    Q_OBJECT

public:
    explicit DijkstraStepper(const Graph& graph, int startVertex);
    ~DijkstraStepper() override;

    bool hasNextStep() const;
    StepChange nextStep();
    const DijkstraStepState& getState() const;

signals:
    void logUpdate(const QString& message);

private:
    const Graph& graph;
    DijkstraStepState state;

    using PQElement = QPair<int, int>;
    struct Compare {
        bool operator()(const PQElement& a, const PQElement& b) const {
            return a.first > b.first;  // min-heap
        }
    };

    std::priority_queue<PQElement, std::vector<PQElement>, Compare> pq;
};

} // namespace graphlib

#endif // DIJKSTRASTEPPER_H
