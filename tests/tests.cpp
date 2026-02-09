#include <catch2/catch_test_macros.hpp>
#include <stdexcept>
#include <vector>

#include "directed_graph.hpp"
#include "graph.hpp"
#include "list_sequence.hpp"
#include "shortest_paths.hpp"

template <typename T>
std::vector<T> ToVector(const SequencePtr<T>& seq) {
    std::vector<T> res;
    if (seq == nullptr) {
        return res;
    }
    auto it = seq->GetIterator();
    for (; it->HasNext(); it->Next()) {
        res.push_back(it->GetCurrentItem());
    }
    return res;
}

std::vector<PathStep> ToPathSteps(const PathSteps& seq) {
    return ToVector(seq);
}

std::vector<size_t> ArcVertices(const Arcs& arcs) {
    std::vector<size_t> res;
    auto it = arcs->GetIterator();
    for (; it->HasNext(); it->Next()) {
        res.push_back(it->GetCurrentItem().vertex->id);
    }
    return res;
}

TEST_CASE("Undirected") {
    Graph g(3);
    g.AddEdge({0, 1, 7});
    g.AddEdge({1, 2, 2});

    REQUIRE(g.GetVertexCount() == 3);
    REQUIRE(g.GetEdgeCount() == 2);

    auto adj0 = ArcVertices(g.GetArcs(0));
    auto adj1 = ArcVertices(g.GetArcs(1));
    auto adj2 = ArcVertices(g.GetArcs(2));

    REQUIRE(adj0 == std::vector<size_t>{1});
    REQUIRE(adj1 == std::vector<size_t>{0, 2});
    REQUIRE(adj2 == std::vector<size_t>{1});
}

TEST_CASE("Directed") {
    auto g = std::make_shared<DirectedGraph>(3);
    g->AddEdge({0, 1, 5});

    REQUIRE(g->GetVertexCount() == 3);
    REQUIRE(g->GetEdgeCount() == 1);

    REQUIRE(ArcVertices(g->GetArcs(0)) == std::vector<size_t>{1});
    REQUIRE(ArcVertices(g->GetArcs(1)).empty());
    REQUIRE(ArcVertices(g->GetArcs(2)).empty());
}

TEST_CASE("Dijkstra") {
    auto edges = std::make_shared<ListSequence<Edge>>();
    edges->Append({0, 1, 4});
    edges->Append({0, 2, 1});
    edges->Append({2, 1, 2});
    edges->Append({1, 3, 1});
    edges->Append({2, 3, 5});

    auto g = std::make_shared<DirectedGraph>(4, edges);
    Dijkstra dijkstra(g, 0);

    REQUIRE(dijkstra.GetDistance(0) == 0);
    REQUIRE(dijkstra.GetDistance(1) == 3);
    REQUIRE(dijkstra.GetDistance(2) == 1);
    REQUIRE(dijkstra.GetDistance(3) == 4);

    REQUIRE(ToVector(dijkstra.GetShortestPath(3)) == std::vector<size_t>{0, 2, 1, 3});
}

TEST_CASE("TrivialPath") {
    auto g = std::make_shared<DirectedGraph>(3);
    g->AddEdge({0, 1, 2});
    Dijkstra d(g, 0);
    REQUIRE(ToVector(d.GetShortestPath(0)) == std::vector<size_t>{0});
}

TEST_CASE("NegEdge") {
    auto g = std::make_shared<DirectedGraph>(2);
    g->AddEdge({0, 1, -1});
    REQUIRE_THROWS_AS(Dijkstra(g, 0), std::invalid_argument);
}

TEST_CASE("BellmanFord") {
    auto edges = std::make_shared<ListSequence<Edge>>();
    edges->Append({0, 1, 1});
    edges->Append({0, 2, 4});
    edges->Append({1, 2, -3});
    edges->Append({2, 3, 2});

    auto g = std::make_shared<DirectedGraph>(4, edges);
    FordBellman bf(g, 0);

    REQUIRE(bf.GetDistance(0) == 0);
    REQUIRE(bf.GetDistance(1) == 1);
    REQUIRE(bf.GetDistance(2) == -2);
    REQUIRE(bf.GetDistance(3) == 0);

    REQUIRE(ToVector(bf.GetShortestPath(3)) == std::vector<size_t>{0, 1, 2, 3});
}

TEST_CASE("Unreachable") {
    auto g = std::make_shared<DirectedGraph>(3);
    g->AddEdge({0, 1, 1});

    Dijkstra d(g, 0);
    REQUIRE(d.GetShortestPath(2) == nullptr);

    FordBellman bf(g, 0);
    REQUIRE(bf.GetShortestPath(2) == nullptr);
}

TEST_CASE("TransportStatefulShortestPath") {
    auto edges = std::make_shared<ListSequence<Edge>>();

    TransferMatrix t01;
    t01.SetCost(Transport::Feet, Transport::Feet, 5);
    t01.SetCost(Transport::Feet, Transport::Bus, 1);
    edges->Append(Edge(0, 1, t01));

    TransferMatrix t12;
    t12.SetCost(Transport::Bus, Transport::Bus, 1);
    t12.SetCost(Transport::Feet, Transport::Feet, 5);
    edges->Append(Edge(1, 2, t12));

    TransferMatrix t02 = TransferMatrix::Diagonal(4);
    edges->Append(Edge(0, 2, t02));

    auto g = std::make_shared<DirectedGraph>(3, edges);

    Dijkstra dijkstra(g, 0);
    REQUIRE(dijkstra.GetDistance(2) == 2);
    REQUIRE(ToVector(dijkstra.GetShortestPath(2)) == std::vector<size_t>{0, 1, 2});
    auto d_path = ToPathSteps(dijkstra.GetShortestPathWithTransfers(2));
    REQUIRE(d_path.size() == 3);
    REQUIRE(d_path[0].vertex == 0);
    REQUIRE(d_path[0].transport == Transport::Feet);
    REQUIRE(d_path[1].vertex == 1);
    REQUIRE(d_path[1].transport == Transport::Bus);
    REQUIRE(d_path[2].vertex == 2);
    REQUIRE(d_path[2].transport == Transport::Bus);

    FordBellman bellman(g, 0);
    REQUIRE(bellman.GetDistance(2) == 2);
    REQUIRE(ToVector(bellman.GetShortestPath(2)) == std::vector<size_t>{0, 1, 2});
    auto b_path = ToPathSteps(bellman.GetShortestPathWithTransfers(2));
    REQUIRE(b_path.size() == 3);
    REQUIRE(b_path[0].vertex == 0);
    REQUIRE(b_path[0].transport == Transport::Feet);
    REQUIRE(b_path[1].vertex == 1);
    REQUIRE(b_path[1].transport == Transport::Bus);
    REQUIRE(b_path[2].vertex == 2);
    REQUIRE(b_path[2].transport == Transport::Bus);
}
