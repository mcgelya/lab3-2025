#include "shortest_paths.hpp"

#include <stdexcept>
#include <vector>

#include "array_sequence.hpp"
#include "igraph.hpp"
#include "list_sequence.hpp"

const int64_t kInf = 1'000'000'000'000'000'000;
const size_t kNoState = kTransportCount;
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

static size_t FindBestStateAtVertex(const SequencePtr<int64_t>& dist, size_t vertex) {
    size_t best_state = kNoState;
    int64_t best_distance = kInf;
    for (Transport transport : kAllTransports) {
        const size_t state = EncodeState(vertex, transport);
        const int64_t candidate = dist->Get(state);
        if (candidate < best_distance) {
            best_distance = candidate;
            best_state = state;
        }
    }
    return best_state;
}

static bool TryAddCost(int64_t distance, int64_t edge_cost, int64_t& out) {
    if (edge_cost > 0 && distance > kInf - edge_cost) {
        return false;
    }
    if (edge_cost < 0 && distance < -kInf - edge_cost) {
        return false;
    }
    out = distance + edge_cost;
    return true;
}

Dijkstra::Dijkstra(IGraphPtr graph, size_t from)
    : dist_(std::make_shared<ArraySequence<int64_t>>(GetStateCount(graph->GetVertexCount()), kInf)),
      prev_(std::make_shared<ArraySequence<size_t>>(GetStateCount(graph->GetVertexCount()), kNoState)),
      from_state_(EncodeState(from, kSourceTransport)),
      vertex_count_(graph->GetVertexCount()) {
    if (from >= vertex_count_) {
        throw std::out_of_range("Source vertex is out of range");
    }
    const size_t state_count = GetStateCount(vertex_count_);
    auto used = std::make_shared<ArraySequence<bool>>(state_count);
    dist_->Set(0, from_state_);

    for (size_t iteration = 0; iteration < state_count; ++iteration) {
        size_t state = kNoState;
        int64_t best_distance = kInf;
        for (size_t s = 0; s < state_count; ++s) {
            if (!used->Get(s) && dist_->Get(s) < best_distance) {
                state = s;
                best_distance = dist_->Get(s);
            }
        }
        if (state == kNoState || best_distance == kInf) {
            break;
        }
        used->Set(true, state);

        const size_t vertex_id = DecodeVertex(state);
        const Transport current_transport = DecodeTransport(state);
        VertexPtr vertex = graph->GetVertex(vertex_id);
        for (auto it = vertex->adjacents->GetIterator(); it->HasNext(); it->Next()) {
            const Adjacent adjacent = it->GetCurrentItem();
            if (adjacent.vertex == nullptr) {
                throw std::runtime_error("Graph contains null adjacent vertex");
            }
            const size_t to_vertex = adjacent.vertex->id;
            for (Transport next_transport : kAllTransports) {
                const int64_t edge_cost = adjacent.transfer.GetCost(current_transport, next_transport);
                if (edge_cost >= kNoTransferCost) {
                    continue;
                }
                if (edge_cost < 0) {
                    throw std::invalid_argument("Dijkstra does not support negative edge weights");
                }
                const size_t to_state = EncodeState(to_vertex, next_transport);
                int64_t candidate = 0;
                if (!TryAddCost(best_distance, edge_cost, candidate)) {
                    continue;
                }
                if (candidate < dist_->Get(to_state)) {
                    dist_->Set(candidate, to_state);
                    prev_->Set(state, to_state);
                }
            }
        }
    }
}

int64_t Dijkstra::GetDistance(size_t to) const {
    if (to >= vertex_count_) {
        throw std::out_of_range("Target vertex is out of range");
    }
    const size_t best_state = FindBestStateAtVertex(dist_, to);
    return best_state == kNoState ? kInf : dist_->Get(best_state);
}

PathSteps Dijkstra::GetShortestPathWithTransfers(size_t to) const {
    if (to >= vertex_count_) {
        throw std::out_of_range("Target vertex is out of range");
    }
    const size_t best_state = FindBestStateAtVertex(dist_, to);
    if (best_state == kNoState || dist_->Get(best_state) == kInf) {
        return nullptr;
    }

    std::vector<size_t> reversed_states;
    for (size_t state = best_state; state != kNoState; state = prev_->Get(state)) {
        reversed_states.push_back(state);
        if (state == from_state_) {
            break;
        }
    }
    if (reversed_states.empty() || reversed_states.back() != from_state_) {
        return nullptr;
    }

    auto res = std::make_shared<ListSequence<PathStep>>();
    for (size_t i = reversed_states.size(); i > 0; --i) {
        const size_t state = reversed_states[i - 1];
        res->Append({DecodeVertex(state), DecodeTransport(state)});
    }
    return res;
}

SequencePtr<size_t> Dijkstra::GetShortestPath(size_t to) const {
    if (to >= vertex_count_) {
        throw std::out_of_range("Target vertex is out of range");
    }
    const size_t best_state = FindBestStateAtVertex(dist_, to);
    if (best_state == kNoState || dist_->Get(best_state) == kInf) {
        return nullptr;
    }

    std::vector<size_t> reversed_states;
    for (size_t state = best_state; state != kNoState; state = prev_->Get(state)) {
        reversed_states.push_back(state);
        if (state == from_state_) {
            break;
        }
    }
    if (reversed_states.empty() || reversed_states.back() != from_state_) {
        return nullptr;
    }

    auto res = std::make_shared<ListSequence<size_t>>();
    for (size_t i = reversed_states.size(); i > 0; --i) {
        const size_t vertex = DecodeVertex(reversed_states[i - 1]);
        if (res->GetLength() == 0 || res->GetLast() != vertex) {
            res->Append(vertex);
        }
    }
    return res;
}

FordBellman::FordBellman(IGraphPtr graph, size_t from)
    : dist_(std::make_shared<ArraySequence<int64_t>>(GetStateCount(graph->GetVertexCount()), kInf)),
      prev_(std::make_shared<ArraySequence<size_t>>(GetStateCount(graph->GetVertexCount()), kNoState)),
      from_state_(EncodeState(from, kSourceTransport)),
      vertex_count_(graph->GetVertexCount()) {
    if (from >= vertex_count_) {
        throw std::out_of_range("Source vertex is out of range");
    }

    const size_t state_count = GetStateCount(vertex_count_);
    dist_->Set(0, from_state_);
    for (size_t iteration = 0; iteration + 1 < state_count; ++iteration) {
        bool updated = false;
        for (size_t state = 0; state < state_count; ++state) {
            const int64_t current_distance = dist_->Get(state);
            if (current_distance == kInf) {
                continue;
            }

            const size_t vertex_id = DecodeVertex(state);
            const Transport current_transport = DecodeTransport(state);
            VertexPtr vertex = graph->GetVertex(vertex_id);
            for (auto it = vertex->adjacents->GetIterator(); it->HasNext(); it->Next()) {
                const Adjacent adjacent = it->GetCurrentItem();
                if (adjacent.vertex == nullptr) {
                    throw std::runtime_error("Graph contains null adjacent vertex");
                }
                const size_t to_vertex = adjacent.vertex->id;
                for (Transport next_transport : kAllTransports) {
                    const int64_t edge_cost = adjacent.transfer.GetCost(current_transport, next_transport);
                    if (edge_cost >= kNoTransferCost) {
                        continue;
                    }
                    const size_t to_state = EncodeState(to_vertex, next_transport);
                    int64_t candidate = 0;
                    if (!TryAddCost(current_distance, edge_cost, candidate)) {
                        continue;
                    }
                    if (candidate < dist_->Get(to_state)) {
                        dist_->Set(candidate, to_state);
                        prev_->Set(state, to_state);
                        updated = true;
                    }
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
    return best_state == kNoState ? kInf : dist_->Get(best_state);
}

PathSteps FordBellman::GetShortestPathWithTransfers(size_t to) const {
    if (to >= vertex_count_) {
        throw std::out_of_range("Target vertex is out of range");
    }
    const size_t best_state = FindBestStateAtVertex(dist_, to);
    if (best_state == kNoState || dist_->Get(best_state) == kInf) {
        return nullptr;
    }

    std::vector<size_t> reversed_states;
    for (size_t state = best_state; state != kNoState; state = prev_->Get(state)) {
        reversed_states.push_back(state);
        if (state == from_state_) {
            break;
        }
    }
    if (reversed_states.empty() || reversed_states.back() != from_state_) {
        return nullptr;
    }

    auto res = std::make_shared<ListSequence<PathStep>>();
    for (size_t i = reversed_states.size(); i > 0; --i) {
        const size_t state = reversed_states[i - 1];
        res->Append({DecodeVertex(state), DecodeTransport(state)});
    }
    return res;
}

SequencePtr<size_t> FordBellman::GetShortestPath(size_t to) const {
    if (to >= vertex_count_) {
        throw std::out_of_range("Target vertex is out of range");
    }
    const size_t best_state = FindBestStateAtVertex(dist_, to);
    if (best_state == kNoState || dist_->Get(best_state) == kInf) {
        return nullptr;
    }

    std::vector<size_t> reversed_states;
    for (size_t state = best_state; state != kNoState; state = prev_->Get(state)) {
        reversed_states.push_back(state);
        if (state == from_state_) {
            break;
        }
    }
    if (reversed_states.empty() || reversed_states.back() != from_state_) {
        return nullptr;
    }

    auto res = std::make_shared<ListSequence<size_t>>();
    for (size_t i = reversed_states.size(); i > 0; --i) {
        const size_t vertex = DecodeVertex(reversed_states[i - 1]);
        if (res->GetLength() == 0 || res->GetLast() != vertex) {
            res->Append(vertex);
        }
    }
    return res;
}
