#include "dijkstra.h"
#include <queue>
#include <limits>
#include <sstream>
#include <algorithm>


DijkstraResult DijkstraAlgorithm::findShortestPaths(const Graph& graph, int startVertex)
{
    int n = graph.verticesCount();
    std::vector<int> dist(n, std::numeric_limits<int>::max());
    std::vector<int> prev(n, -1);

    dist[startVertex] = 0;

    using P = std::pair<int, int>; // (distance, vertex)
    std::priority_queue<P, std::vector<P>, std::greater<P>> pq;

    pq.push({0, startVertex});

    while (!pq.empty()) {
        auto [currentDist, u] = pq.top();
        pq.pop();

        if (currentDist > dist[u])
            continue;

        for (const auto& edge : graph.getEdges(u)) {
            int v = edge.first;
            int weight = edge.second;

            if (dist[u] + weight < dist[v]) {
                dist[v] = dist[u] + weight;
                prev[v] = u;
                pq.push({dist[v], v});
            }
        }
    }

    return DijkstraResult{dist, prev};
}

bool DijkstraAlgorithm::hasPath(const DijkstraResult& result, int to)
{
    return result.distances[to] != std::numeric_limits<int>::max();
}

std::string DijkstraAlgorithm::getPathString(const DijkstraResult& result, int to)
{
    if (!hasPath(result, to))
        return "Путь не существует";

    std::vector<int> path;
    for (int at = to; at != -1; at = result.previous[at]) {
        path.push_back(at);
    }
    std::reverse(path.begin(), path.end());

    std::ostringstream oss;
    for (size_t i = 0; i < path.size(); ++i) {
        oss << (path[i] + 1); // +1 для удобства пользователя (нумерация с 1)
        if (i + 1 < path.size())
            oss << " \u2192 "; // стрелка →
    }
    return oss.str();
}
