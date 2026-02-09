#pragma once

#include "igraph.hpp"

class Graph : public IGraph {
public:
    explicit Graph(size_t n);

    Graph(size_t n, SequencePtr<Edge> edges);

    size_t GetVertexCount() const override;

    size_t GetEdgeCount() const override;

    void AddEdge(const Edge& edge) override;

    VertexPtr GetVertex(size_t v) const override;

    Arcs GetArcs(size_t v) const override;

private:
    SequencePtr<VertexPtr> vertices_;
    size_t edge_count_ = 0;
};
