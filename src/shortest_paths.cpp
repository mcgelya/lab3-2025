#include "shortest_paths.hpp"

#include <stdexcept>

#include "array_sequence.hpp"
#include "igraph.hpp"
#include "list_sequence.hpp"

constexpr int64_t kInf = 1000000000000000000;

Dijkstra::Dijkstra(IGraphPtr graph, size_t from)
    : dist_(std::make_shared<ArraySequence<int64_t>>(graph->GetVertexCount(), kInf)),
      prev_(std::make_shared<ArraySequence<size_t>>(graph->GetVertexCount())),
      from_(from) {
    size_t n = graph->GetVertexCount();
    if (from >= n) {
        throw std::out_of_range("Source vertex is out of range");
    }
    auto used = std::make_shared<ArraySequence<bool>>(n);
    dist_->Set(0, from);
    for (size_t iteration = 0; iteration < n; ++iteration) {
        size_t u = 0;
        int64_t mn_dist = kInf;
        for (size_t v = 0; v < n; ++v) {
            if (!used->Get(v) && dist_->Get(v) < mn_dist) {
                u = v;
                mn_dist = dist_->Get(v);
            }
        }
        if (mn_dist == kInf) {
            break;
        }
        used->Set(true, u);
        VertexPtr vertex = graph->GetVertex(u);
        for (auto it = vertex->adjacents->GetIterator(); it->HasNext(); it->Next()) {
            auto cur = it->GetCurrentItem();
            if (cur.vertex == nullptr) {
                throw std::runtime_error("Graph contains null adjacent vertex");
            }
            if (cur.w < 0) {
                throw std::invalid_argument("Dijkstra does not support negative edge weights");
            }
            const size_t to = cur.vertex->id;
            if (mn_dist + cur.w < dist_->Get(to)) {
                dist_->Set(mn_dist + cur.w, to);
                prev_->Set(u, to);
            }
        }
    }
}

int64_t Dijkstra::GetDistance(size_t to) const {
    return dist_->Get(to);
}

SequencePtr<size_t> Dijkstra::GetShortestPath(size_t to) const {
    if (dist_->Get(to) == kInf) {
        return nullptr;
    }
    auto res = std::make_shared<ListSequence<size_t>>();
    res->Prepend(to);
    while (to != from_) {
        to = prev_->Get(to);
        res->Prepend(to);
    }
    return res;
}

FordBellman::FordBellman(IGraphPtr graph, size_t from)
    : dist_(std::make_shared<ArraySequence<int64_t>>(graph->GetVertexCount(), kInf)),
      prev_(std::make_shared<ArraySequence<size_t>>(graph->GetVertexCount())),
      from_(from) {
    if (from >= graph->GetVertexCount()) {
        throw std::out_of_range("Source vertex is out of range");
    }
    size_t n = graph->GetVertexCount();
    dist_->Set(0, from);
    for (size_t iteration = 0; iteration + 1 < n; ++iteration) {
        bool updated = false;
        for (size_t u = 0; u < n; ++u) {
            int64_t du = dist_->Get(u);
            if (du == kInf) {
                continue;
            }
            VertexPtr vertex = graph->GetVertex(u);
            for (auto it = vertex->adjacents->GetIterator(); it->HasNext(); it->Next()) {
                auto cur = it->GetCurrentItem();
                if (cur.vertex == nullptr) {
                    throw std::runtime_error("Graph contains null adjacent vertex");
                }
                const size_t to = cur.vertex->id;
                if (du + cur.w < dist_->Get(to)) {
                    dist_->Set(du + cur.w, to);
                    prev_->Set(u, to);
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
    return dist_->Get(to);
}

SequencePtr<size_t> FordBellman::GetShortestPath(size_t to) const {
    if (dist_->Get(to) == kInf) {
        return nullptr;
    }
    auto res = std::make_shared<ListSequence<size_t>>();
    res->Prepend(to);
    while (to != from_) {
        to = prev_->Get(to);
        res->Prepend(to);
    }
    return res;
}
