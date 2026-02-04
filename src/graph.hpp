#pragma once

#include "igraph.hpp"

class Graph : public IGraph {
public:
    explicit Graph(size_t n);

    Graph(size_t n, SequencePtr<Edge> edges);

    size_t GetSize() const override;

    void AddEdge(const Edge& edge) override;

    Adjacents GetAdjacent(size_t v) const override;

private:
    SequencePtr<Adjacents> graph_;
};
