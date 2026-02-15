#include "shortest_paths.hpp"

#include <stdexcept>

#include "array_sequence.hpp"
#include "igraph.hpp"
#include "list_sequence.hpp"

const int64_t kInf = 1'000'000'000'000'000'000;
const size_t kNoState = kInf;
const Transport kSourceTransport = Transport::Feet;

static size_t EncodeState(size_t vertex, Transport transport) {
    return vertex * kTransportCount + ToTransportIndex(transport);
}

static size_t DecodeVertex(size_t state) {
    return state / kTransportCount;
}

static Transport DecodeTransport(size_t state) {
    return static_cast<Transport>(state % kTransportCount);
}

static size_t GetStateCount(size_t vertex_count) {
    return vertex_count * kTransportCount;
}

static bool CombineTransfer(
    const TransferMatrix& transfer, const AccumulatedPath& current, Transport from_transport, Transport to_transport,
    AccumulatedPath& combined) {
    const int64_t step_cost = transfer.GetCost(from_transport, to_transport);
    if (step_cost >= kNoTransferCost) {
        return false;
    }
    return current.Combine(step_cost, combined);
}

static size_t FindBestStateAtVertex(const SequencePtr<AccumulatedPath>& dist, size_t vertex) {
    size_t best_state = kNoState;
    int64_t best_distance = kInf;
    for (Transport transport : kAllTransports) {
        const size_t state = EncodeState(vertex, transport);
        const int64_t candidate = dist->Get(state).total_cost;
        if (candidate < best_distance) {
            best_distance = candidate;
            best_state = state;
        }
    }
    return best_state;
}

Dijkstra::Dijkstra(IGraphPtr graph, size_t from)
    : dist_(std::make_shared<ArraySequence<AccumulatedPath>>(
          GetStateCount(graph->GetVertexCount()), AccumulatedPath{kInf})),
      prev_(std::make_shared<ArraySequence<size_t>>(GetStateCount(graph->GetVertexCount()), kNoState)),
      from_state_(EncodeState(from, kSourceTransport)),
      vertex_count_(graph->GetVertexCount()) {
    if (from >= vertex_count_) {
        throw std::out_of_range("Source vertex is out of range");
    }
    const size_t state_count = GetStateCount(vertex_count_);
    auto used = std::make_shared<ArraySequence<bool>>(state_count);
    dist_->Set(AccumulatedPath{0}, from_state_);

    for (size_t iteration = 0; iteration < state_count; ++iteration) {
        size_t state = kNoState;
        int64_t best_distance = kInf;
        for (size_t s = 0; s < state_count; ++s) {
            if (!used->Get(s) && dist_->Get(s).total_cost < best_distance) {
                state = s;
                best_distance = dist_->Get(s).total_cost;
            }
        }
        if (state == kNoState || best_distance == kInf) {
            break;
        }
        used->Set(true, state);

        const AccumulatedPath current = dist_->Get(state);
        const size_t vertex_id = DecodeVertex(state);
        const Transport current_transport = DecodeTransport(state);
        VertexPtr vertex = graph->GetVertex(vertex_id);

        for (Transport next_transport : kAllTransports) {
            const size_t to_state = EncodeState(vertex_id, next_transport);
            AccumulatedPath candidate;
            if (!CombineTransfer(vertex->transfer, current, current_transport, next_transport, candidate)) {
                continue;
            }
            if (candidate.total_cost < best_distance) {
                throw std::invalid_argument("Dijkstra does not support negative edge weights");
            }
            if (candidate.total_cost < dist_->Get(to_state).total_cost) {
                dist_->Set(candidate, to_state);
                prev_->Set(state, to_state);
            }
        }

        for (auto it = vertex->arcs->GetIterator(); it->HasNext(); it->Next()) {
            const Arc arc = it->GetCurrentItem();
            if (arc.vertex == nullptr) {
                throw std::runtime_error("Graph contains null adjacent vertex");
            }
            const size_t to_vertex = arc.vertex->id;
            const size_t to_state = EncodeState(to_vertex, current_transport);
            AccumulatedPath candidate;
            if (!current.Combine(arc.weight, candidate)) {
                continue;
            }
            if (candidate.total_cost < best_distance) {
                throw std::invalid_argument("Dijkstra does not support negative edge weights");
            }
            if (candidate.total_cost < dist_->Get(to_state).total_cost) {
                dist_->Set(candidate, to_state);
                prev_->Set(state, to_state);
            }
        }
    }
}

int64_t Dijkstra::GetDistance(size_t to) const {
    if (to >= vertex_count_) {
        throw std::out_of_range("Target vertex is out of range");
    }
    const size_t best_state = FindBestStateAtVertex(dist_, to);
    return best_state == kNoState ? kInf : dist_->Get(best_state).total_cost;
}

PathSteps Dijkstra::GetShortestPathWithTransfers(size_t to) const {
    if (to >= vertex_count_) {
        throw std::out_of_range("Target vertex is out of range");
    }
    const size_t best_state = FindBestStateAtVertex(dist_, to);
    if (best_state == kNoState || dist_->Get(best_state).total_cost == kInf) {
        return nullptr;
    }

    auto res = std::make_shared<ListSequence<PathStep>>();
    bool reached_source = false;
    for (size_t state = best_state; state != kNoState; state = prev_->Get(state)) {
        const size_t prev_state = prev_->Get(state);
        const bool is_transfer = prev_state != kNoState && DecodeVertex(prev_state) == DecodeVertex(state) &&
                                 DecodeTransport(prev_state) != DecodeTransport(state);
        res->Prepend({DecodeVertex(state), DecodeTransport(state), is_transfer});
        if (state == from_state_) {
            reached_source = true;
            break;
        }
    }
    if (!reached_source) {
        return nullptr;
    }
    return res;
}

SequencePtr<size_t> Dijkstra::GetShortestPath(size_t to) const {
    PathSteps detailed = GetShortestPathWithTransfers(to);
    if (detailed == nullptr) {
        return nullptr;
    }

    auto res = std::make_shared<ListSequence<size_t>>();
    for (auto it = detailed->GetIterator(); it->HasNext(); it->Next()) {
        const size_t vertex = it->GetCurrentItem().vertex;
        if (res->GetLength() == 0 || res->GetLast() != vertex) {
            res->Append(vertex);
        }
    }
    return res;
}

FordBellman::FordBellman(IGraphPtr graph, size_t from)
    : dist_(std::make_shared<ArraySequence<AccumulatedPath>>(
          GetStateCount(graph->GetVertexCount()), AccumulatedPath{kInf})),
      prev_(std::make_shared<ArraySequence<size_t>>(GetStateCount(graph->GetVertexCount()), kNoState)),
      from_state_(EncodeState(from, kSourceTransport)),
      vertex_count_(graph->GetVertexCount()) {
    if (from >= vertex_count_) {
        throw std::out_of_range("Source vertex is out of range");
    }

    const size_t state_count = GetStateCount(vertex_count_);
    dist_->Set(AccumulatedPath{0}, from_state_);
    for (size_t iteration = 0; iteration + 1 < state_count; ++iteration) {
        bool updated = false;
        for (size_t state = 0; state < state_count; ++state) {
            const AccumulatedPath current = dist_->Get(state);
            if (current.total_cost == kInf) {
                continue;
            }

            const size_t vertex_id = DecodeVertex(state);
            const Transport current_transport = DecodeTransport(state);
            VertexPtr vertex = graph->GetVertex(vertex_id);

            for (Transport next_transport : kAllTransports) {
                const size_t to_state = EncodeState(vertex_id, next_transport);
                AccumulatedPath candidate;
                if (!CombineTransfer(vertex->transfer, current, current_transport, next_transport, candidate)) {
                    continue;
                }
                if (candidate.total_cost < dist_->Get(to_state).total_cost) {
                    dist_->Set(candidate, to_state);
                    prev_->Set(state, to_state);
                    updated = true;
                }
            }

            for (auto it = vertex->arcs->GetIterator(); it->HasNext(); it->Next()) {
                const Arc arc = it->GetCurrentItem();
                if (arc.vertex == nullptr) {
                    throw std::runtime_error("Graph contains null adjacent vertex");
                }
                const size_t to_vertex = arc.vertex->id;
                const size_t to_state = EncodeState(to_vertex, current_transport);
                AccumulatedPath candidate;
                if (!current.Combine(arc.weight, candidate)) {
                    continue;
                }
                if (candidate.total_cost < dist_->Get(to_state).total_cost) {
                    dist_->Set(candidate, to_state);
                    prev_->Set(state, to_state);
                    updated = true;
                }
            }
        }
        if (!updated) {
            break;
        }
    }
}

int64_t FordBellman::GetDistance(size_t to) const {
    if (to >= vertex_count_) {
        throw std::out_of_range("Target vertex is out of range");
    }
    const size_t best_state = FindBestStateAtVertex(dist_, to);
    return best_state == kNoState ? kInf : dist_->Get(best_state).total_cost;
}

PathSteps FordBellman::GetShortestPathWithTransfers(size_t to) const {
    if (to >= vertex_count_) {
        throw std::out_of_range("Target vertex is out of range");
    }
    const size_t best_state = FindBestStateAtVertex(dist_, to);
    if (best_state == kNoState || dist_->Get(best_state).total_cost == kInf) {
        return nullptr;
    }

    auto res = std::make_shared<ListSequence<PathStep>>();
    bool reached_source = false;
    for (size_t state = best_state; state != kNoState; state = prev_->Get(state)) {
        const size_t prev_state = prev_->Get(state);
        const bool is_transfer = prev_state != kNoState && DecodeVertex(prev_state) == DecodeVertex(state) &&
                                 DecodeTransport(prev_state) != DecodeTransport(state);
        res->Prepend({DecodeVertex(state), DecodeTransport(state), is_transfer});
        if (state == from_state_) {
            reached_source = true;
            break;
        }
    }
    if (!reached_source) {
        return nullptr;
    }
    return res;
}

SequencePtr<size_t> FordBellman::GetShortestPath(size_t to) const {
    PathSteps detailed = GetShortestPathWithTransfers(to);
    if (detailed == nullptr) {
        return nullptr;
    }

    auto res = std::make_shared<ListSequence<size_t>>();
    for (auto it = detailed->GetIterator(); it->HasNext(); it->Next()) {
        const size_t vertex = it->GetCurrentItem().vertex;
        if (res->GetLength() == 0 || res->GetLast() != vertex) {
            res->Append(vertex);
        }
    }
    return res;
}
