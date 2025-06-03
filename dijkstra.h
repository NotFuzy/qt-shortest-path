#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "graph.h"
#include <QString>
#include <QVector>

namespace graphlib {

struct DijkstraResult {
    QVector<int> distances;
    QVector<int> previous;
};

class DijkstraAlgorithm
{
public:
    static DijkstraResult findShortestPaths(const Graph& graph, int startVertex);
    static bool hasPath(const DijkstraResult& result, int to);
    static QString getPathString(const DijkstraResult& result, int to);
};

} // namespace graphlib

#endif // DIJKSTRA_H
