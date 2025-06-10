#include "dijkstrastepper.h"
#include <limits>

namespace graphlib {

// Реализация конструктора
DijkstraStepper::DijkstraStepper(const Graph& graph, int startVertex)
    : graph(graph)
{
    int n = graph.verticesCount();
    state.distances.fill(std::numeric_limits<int>::max(), n);
    state.previous.fill(-1, n);
    state.distances[startVertex] = 0;

    pq.push(qMakePair(0, startVertex));
}

// Реализация виртуального деструктора
DijkstraStepper::~DijkstraStepper() = default;

bool DijkstraStepper::hasNextStep() const {
    return !pq.empty();
}

StepChange DijkstraStepper::nextStep() {
    StepChange change;

    while (!pq.empty()) {
        auto [curDist, u] = pq.top();
        pq.pop();

        if (state.visited.contains(u)) {
            change.currentVertex = -1;  // сигнал, что шаг не нужен
            return change;
        }

        state.visited.insert(u);
        state.currentVertex = u;

        change.currentVertex = u;
        change.highlightVertices.append(u);

        for (const auto& [v, weight] : graph.getEdges(u)) {
            if (state.visited.contains(v)) continue;

            int newDist = state.distances[u] + weight;
            if (newDist < state.distances[v]) {
                change.vertexUpdated = v;
                change.oldDistance = state.distances[v];
                change.newDistance = newDist;
                change.fromVertex = u;

                state.distances[v] = newDist;
                state.previous[v] = u;
                pq.push(qMakePair(newDist, v));

                change.highlightVertices.append(v);
                change.highlightEdges.append(qMakePair(u, v));
            }
        }

        return change; // только первый непосещённый шаг
    }

    return change; // очередь пуста, завершение
}


const DijkstraStepState& DijkstraStepper::getState() const {
    return state;
}

} // namespace graphlib
