#pragma once

#include "igraph.hpp"

class DirectedGraph : public IGraph {
public:
    explicit DirectedGraph(size_t n);

    DirectedGraph(size_t n, SequencePtr<Edge> edges);

    size_t GetVertexCount() const override;

    size_t GetEdgeCount() const override;

    void AddEdge(const Edge& edge) override;

    VertexPtr GetVertex(size_t v) const override;

    Adjacents GetAdjacent(size_t v) const override;

private:
    SequencePtr<VertexPtr> vertices_;
    size_t edge_count_ = 0;
};
