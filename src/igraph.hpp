#pragma once

#include "fwd.hpp"
#include "list_sequence.hpp"

struct Vertex;

using VertexPtr = std::shared_ptr<Vertex>;

struct Edge {
    size_t u, v;
    int64_t w = 1;
};

struct Adjacent {
    VertexPtr vertex;
    int64_t w;
};

using Adjacents = SequencePtr<Adjacent>;

struct Vertex {
    explicit Vertex(size_t id_) : id(id_), adjacents(std::make_shared<ListSequence<Adjacent>>()) {
    }

    Vertex() : Vertex(0) {
    }

    size_t id;
    Adjacents adjacents;
};

class IGraph {
public:
    virtual ~IGraph() = default;

    virtual size_t GetVertexCount() const = 0;

    virtual size_t GetEdgeCount() const = 0;

    virtual void AddEdge(const Edge& edge) = 0;

    virtual VertexPtr GetVertex(size_t v) const = 0;

    virtual Adjacents GetAdjacent(size_t v) const = 0;
};
