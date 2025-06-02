#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <utility>

class Graph
{
public:
    explicit Graph(int vertices);
    void addEdge(int from, int to, int weight);
    int verticesCount() const;

    // Для доступа к ребрам
    const std::vector<std::pair<int, int>>& getEdges(int vertex) const;

private:
    int m_vertices;
    // Для каждой вершины список пар (смежная вершина, вес ребра)
    std::vector<std::vector<std::pair<int, int>>> adjacencyList;
};

#endif // GRAPH_H
