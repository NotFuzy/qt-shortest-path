#ifndef GRAPH_H
#define GRAPH_H

#include <QObject>
#include <QVector>
#include <QPair>

namespace graphlib {

class Graph : public QObject
{
    Q_OBJECT

public:
    Graph(int vertices, bool directed = false, QObject* parent = nullptr);

    void addEdge(int from, int to, int weight);
    void clearGraph();
    void addVertex();
    int verticesCount() const;
    bool isDirected() const;
    void setDirected(bool directed);
    const QVector<QPair<int, int>>& getEdges(int vertex) const;

signals:
    void graphModified();

private:
    QVector<QVector<QPair<int, int>>> adjacencyList;
    int m_vertices;
    bool m_directed;

    bool vertexExists(int v) const;
};

} // namespace graphlib

#endif // GRAPH_H
