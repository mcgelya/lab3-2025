#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>

#include "fwd.hpp"
#include "list_sequence.hpp"

enum class Transport : uint8_t {
    Bus = 0,
    Car = 1,
    Feet = 2,
};

constexpr size_t kTransportCount = 3;

constexpr size_t ToTransportIndex(Transport transport) {
    return static_cast<size_t>(transport);
}

constexpr std::array<Transport, kTransportCount> kAllTransports = {
    Transport::Bus,
    Transport::Car,
    Transport::Feet,
};

constexpr int64_t kNoTransferCost = 1'000'000'000'000'000'000;

struct TransferMatrix {
    std::array<std::array<int64_t, kTransportCount>, kTransportCount> cost{};

    TransferMatrix() {
        for (auto& row : cost) {
            row.fill(kNoTransferCost);
        }
    }

    static TransferMatrix Diagonal(int64_t diagonal_cost) {
        TransferMatrix matrix;
        for (size_t i = 0; i < kTransportCount; ++i) {
            matrix.cost[i][i] = diagonal_cost;
        }
        return matrix;
    }

    static TransferMatrix Uniform(int64_t c) {
        TransferMatrix matrix;
        for (auto& row : matrix.cost) {
            row.fill(c);
        }
        return matrix;
    }

    int64_t GetCost(Transport from, Transport to) const {
        return cost[ToTransportIndex(from)][ToTransportIndex(to)];
    }

    void SetCost(Transport from, Transport to, int64_t c) {
        cost[ToTransportIndex(from)][ToTransportIndex(to)] = c;
    }
};

struct Edge {
    Edge(size_t u_, size_t v_, int64_t w = 1) : u(u_), v(v_), weight(w) {
    }

    size_t u;
    size_t v;
    int64_t weight;
};

struct AccumulatedPath {
    int64_t total_cost = 0;

    bool Combine(int64_t delta_cost, AccumulatedPath& combined) const {
        if (delta_cost > 0 && total_cost > std::numeric_limits<int64_t>::max() - delta_cost) {
            return false;
        }
        if (delta_cost < 0 && total_cost < std::numeric_limits<int64_t>::min() - delta_cost) {
            return false;
        }
        combined = *this;
        combined.total_cost = total_cost + delta_cost;
        return true;
    }
};

struct Arc {
    VertexPtr from;
    VertexPtr vertex;
    int64_t weight;
};

using Arcs = SequencePtr<Arc>;

struct Vertex {
    explicit Vertex(size_t id_, const TransferMatrix& transfer_ = TransferMatrix::Diagonal(0))
        : id(id_), transfer(transfer_), arcs(std::make_shared<ListSequence<Arc>>()) {
    }

    Vertex() : Vertex(0) {
    }

    size_t id;
    TransferMatrix transfer;
    Arcs arcs;
};

class IGraph {
public:
    virtual ~IGraph() = default;

    virtual size_t GetVertexCount() const = 0;

    virtual size_t GetEdgeCount() const = 0;

    virtual void AddEdge(const Edge& edge) = 0;

    virtual VertexPtr GetVertex(size_t v) const = 0;

    virtual Arcs GetArcs(size_t v) const = 0;
};
