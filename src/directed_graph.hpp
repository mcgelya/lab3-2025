#pragma once

#include "igraph.hpp"

class DirectedGraph : public IGraph {
public:
    explicit DirectedGraph(size_t n);

    DirectedGraph(size_t n, SequencePtr<Edge> edges);

    size_t GetSize() const override;

    void AddEdge(const Edge& edge) override;

    Adjacents GetAdjacent(size_t v) const override;

private:
    SequencePtr<Adjacents> graph_;
};
