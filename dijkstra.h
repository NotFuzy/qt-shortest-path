#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "graph.h"
#include <vector>
#include <string>

struct DijkstraResult {
    std::vector<int> distances;
    std::vector<int> previous; // для восстановления пути
};

class DijkstraAlgorithm
{
public:
    static DijkstraResult findShortestPaths(const Graph& graph, int startVertex);
    static bool hasPath(const DijkstraResult& result, int to);
    static std::string getPathString(const DijkstraResult& result, int to);
};

#endif // DIJKSTRA_H
