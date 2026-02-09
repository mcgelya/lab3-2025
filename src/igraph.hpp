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
    Edge(size_t u_, size_t v_, int64_t w = 1) : u(u_), v(v_), transfer(TransferMatrix::Diagonal(w)) {
    }

    Edge(size_t u_, size_t v_, const TransferMatrix& transfer_) : u(u_), v(v_), transfer(transfer_) {
    }

    size_t u;
    size_t v;
    TransferMatrix transfer;
};

struct Arc {
    VertexPtr vertex;
    TransferMatrix transfer;

    bool Combine(int64_t current_weight, Transport from, Transport to, int64_t& combined_weight) const {
        const int64_t step_cost = transfer.GetCost(from, to);
        if (step_cost >= kNoTransferCost) {
            return false;
        }
        if (step_cost > 0 && current_weight > std::numeric_limits<int64_t>::max() - step_cost) {
            return false;
        }
        if (step_cost < 0 && current_weight < std::numeric_limits<int64_t>::min() - step_cost) {
            return false;
        }
        combined_weight = current_weight + step_cost;
        return true;
    }
};

using Arcs = SequencePtr<Arc>;

struct Vertex {
    explicit Vertex(size_t id_) : id(id_), arcs(std::make_shared<ListSequence<Arc>>()) {
    }

    Vertex() : Vertex(0) {
    }

    size_t id;
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
