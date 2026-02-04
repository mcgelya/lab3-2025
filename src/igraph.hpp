#pragma once

#include "fwd.hpp"

struct Edge {
    size_t u, v;
    int64_t w = 1;
};

struct Adjacent {
    size_t v;
    int64_t w;
};

using Adjacents = SequencePtr<Adjacent>;

class IGraph {
public:
    virtual ~IGraph() = default;

    virtual size_t GetSize() const = 0;

    virtual void AddEdge(const Edge& edge) = 0;

    virtual Adjacents GetAdjacent(size_t v) const = 0;
};
