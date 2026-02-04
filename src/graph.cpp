#include "graph.hpp"

#include "array_sequence.hpp"
#include "list_sequence.hpp"

Graph::Graph(size_t n) : graph_(std::make_shared<ArraySequence<Adjacents>>(n)) {
}

Graph::Graph(size_t n, SequencePtr<Edge> edges) : graph_(std::make_shared<ArraySequence<Adjacents>>(n)) {
    for (auto it = edges->GetIterator(); it->HasNext(); it->Next()) {
        AddEdge(it->GetCurrentItem());
    }
}

size_t Graph::GetSize() const {
    return graph_->GetLength();  // вершины
}

void Graph::AddEdge(const Edge& edge) {
    {
        auto adjs = graph_->Get(edge.u);
        if (adjs == nullptr) {
            adjs = std::make_shared<ListSequence<Adjacent>>();
            graph_->Set(adjs, edge.u);
        }
        adjs->Append({edge.v, edge.w});
    }
    {
        auto adjs = graph_->Get(edge.v);
        if (adjs == nullptr) {
            adjs = std::make_shared<ListSequence<Adjacent>>();
            graph_->Set(adjs, edge.v);
        }
        adjs->Append({edge.u, edge.w});
    }
}

Adjacents Graph::GetAdjacent(size_t v) const {
    auto adjs = graph_->Get(v);
    if (adjs == nullptr) {
        adjs = std::make_shared<ListSequence<Adjacent>>();
        graph_->Set(adjs, v);
    }
    return adjs;
}
