#include "directed_graph.hpp"

#include <stdexcept>

#include "array_sequence.hpp"

DirectedGraph::DirectedGraph(size_t n) : vertices_(std::make_shared<ArraySequence<VertexPtr>>(n)) {
    for (size_t i = 0; i < n; ++i) {
        vertices_->Set(std::make_shared<Vertex>(i), i);
    }
}

DirectedGraph::DirectedGraph(size_t n, SequencePtr<Edge> edges) : DirectedGraph(n) {
    for (auto it = edges->GetIterator(); it->HasNext(); it->Next()) {
        AddEdge(it->GetCurrentItem());
    }
}

size_t DirectedGraph::GetVertexCount() const {
    return vertices_->GetLength();
}

size_t DirectedGraph::GetEdgeCount() const {
    return edge_count_;
}

void DirectedGraph::AddEdge(const Edge& edge) {
    if (edge.u >= GetVertexCount() || edge.v >= GetVertexCount()) {
        throw std::out_of_range("Vertex index is out of range");
    }
    VertexPtr from = GetVertex(edge.u);
    VertexPtr to = GetVertex(edge.v);
    from->arcs->Append({to, edge.transfer});
    ++edge_count_;
}

VertexPtr DirectedGraph::GetVertex(size_t v) const {
    return vertices_->Get(v);
}

Arcs DirectedGraph::GetArcs(size_t v) const {
    return GetVertex(v)->arcs;
}
