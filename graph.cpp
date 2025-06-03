#include "graph.h"
#include <algorithm>

namespace graphlib {

Graph::Graph(int vertices, bool directed, QObject* parent)
    : QObject(parent), m_vertices(vertices), m_directed(directed)
{
    adjacencyList.resize(vertices);
}

void Graph::addEdge(int from, int to, int weight)
{
    if (!vertexExists(from) || !vertexExists(to))
        return;

    adjacencyList[from].append(qMakePair(to, weight));

    if (!m_directed && from != to) {
        adjacencyList[to].append(qMakePair(from, weight));
    }

    emit graphModified();
}

void Graph::clearGraph()
{
    for (auto& edges : adjacencyList)
        edges.clear();

    emit graphModified();
}

int Graph::verticesCount() const { return m_vertices; }

bool Graph::isDirected() const { return m_directed; }

void Graph::setDirected(bool directed)
{
    if (m_directed == directed)
        return;

    // Конвертация рёбер при смене направленности
    if (!directed && m_directed) {
        for (int u = 0; u < m_vertices; ++u) {
            for (const auto& edge : adjacencyList[u]) {
                int v = edge.first;
                int w = edge.second;

                auto& reverseEdges = adjacencyList[v];
                if (std::none_of(reverseEdges.begin(), reverseEdges.end(),
                    [u](const QPair<int, int>& p) { return p.first == u; })) {
                    reverseEdges.append(qMakePair(u, w));
                }
            }
        }
    } else if (directed && !m_directed) {
        for (int u = 0; u < m_vertices; ++u) {
            QVector<QPair<int, int>> filtered;
            for (const auto& edge : adjacencyList[u]) {
                if (u < edge.first) {
                    filtered.append(edge);
                }
            }
            adjacencyList[u] = filtered;
        }
    }

    m_directed = directed;
    emit graphModified();
}

bool Graph::vertexExists(int v) const
{
    return v >= 0 && v < m_vertices;
}

const QVector<QPair<int, int>>& Graph::getEdges(int vertex) const
{
    static QVector<QPair<int, int>> empty;
    return vertexExists(vertex) ? adjacencyList[vertex] : empty;
}

} // namespace graphlib
