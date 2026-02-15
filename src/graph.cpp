#include "graph.hpp"

#include <stdexcept>

#include "array_sequence.hpp"

Graph::Graph(size_t n) : vertices_(std::make_shared<ArraySequence<VertexPtr>>(n)) {
    for (size_t i = 0; i < n; ++i) {
        vertices_->Set(std::make_shared<Vertex>(i), i);
    }
}

Graph::Graph(size_t n, SequencePtr<Edge> edges) : Graph(n) {
    for (auto it = edges->GetIterator(); it->HasNext(); it->Next()) {
        AddEdge(it->GetCurrentItem());
    }
}

size_t Graph::GetVertexCount() const {
    return vertices_->GetLength();
}

size_t Graph::GetEdgeCount() const {
    return edge_count_;
}

void Graph::AddEdge(const Edge& edge) {
    if (edge.u >= GetVertexCount() || edge.v >= GetVertexCount()) {
        throw std::out_of_range("Vertex index is out of range");
    }
    VertexPtr from = GetVertex(edge.u);
    VertexPtr to = GetVertex(edge.v);
    from->arcs->Append({from, to, edge.weight});
    to->arcs->Append({to, from, edge.weight});
    ++edge_count_;
}

VertexPtr Graph::GetVertex(size_t v) const {
    return vertices_->Get(v);
}

Arcs Graph::GetArcs(size_t v) const {
    return GetVertex(v)->arcs;
}
