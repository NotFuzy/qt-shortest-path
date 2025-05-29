#include "graph.h"

Graph::Graph(int vertices)
    : m_vertices(vertices)
{
    adjacencyList.resize(vertices);
}

void Graph::addEdge(int from, int to, int weight)
{
    adjacencyList[from].push_back({to, weight});
}

int Graph::verticesCount() const
{
    return m_vertices;
}

const std::vector<std::pair<int, int>>& Graph::getEdges(int vertex) const
{
    return adjacencyList[vertex];
}
