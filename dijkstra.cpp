#include "dijkstra.h"
#include <QQueue>
#include <QSet>
#include <QStringList>
#include <limits>
#include <queue>
#include <algorithm>

namespace graphlib {

DijkstraResult DijkstraAlgorithm::findShortestPaths(const Graph& graph, int startVertex)
{
    int n = graph.verticesCount();
    QVector<int> dist(n, std::numeric_limits<int>::max());
    QVector<int> prev(n, -1);
    dist[startVertex] = 0;

    using P = QPair<int, int>; // (distance, vertex)
    auto cmp = [](const P& a, const P& b) { return a.first > b.first; };
    std::priority_queue<P, std::vector<P>, decltype(cmp)> pq(cmp);

    pq.push(qMakePair(0, startVertex));

    while (!pq.empty()) {
        auto [curDist, u] = pq.top(); pq.pop();
        if (curDist > dist[u]) continue;

        for (const auto& [v, weight] : graph.getEdges(u)) {
            if (dist[u] + weight < dist[v]) {
                dist[v] = dist[u] + weight;
                prev[v] = u;
                pq.push(qMakePair(dist[v], v));
            }
        }
    }

    return { dist, prev };
}

bool DijkstraAlgorithm::hasPath(const DijkstraResult& result, int to)
{
    return result.distances[to] != std::numeric_limits<int>::max();
}

QString DijkstraAlgorithm::getPathString(const DijkstraResult& result, int to)
{
    if (!hasPath(result, to))
        return "Путь не существует";

    QVector<int> path;
    for (int at = to; at != -1; at = result.previous[at])
        path.append(at);

    std::reverse(path.begin(), path.end());

    QStringList out;
    for (int v : path)
        out << QString::number(v + 1); // assuming UI is 1-based

    return out.join(" → ");
}

} // namespace graphlib
