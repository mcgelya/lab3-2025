#pragma once

#include "ishortest_paths.hpp"

class Dijkstra : public IShortestPathsFinder {
public:
    Dijkstra(IGraphPtr graph, size_t from);

    int64_t GetDistance(size_t to) const override;

    SequencePtr<size_t> GetShortestPath(size_t to) const override;

    PathSteps GetShortestPathWithTransfers(size_t to) const override;

private:
    SequencePtr<AccumulatedPath> dist_;
    SequencePtr<size_t> prev_;
    size_t from_state_;
    size_t vertex_count_;
};

class FordBellman : public IShortestPathsFinder {
public:
    FordBellman(IGraphPtr graph, size_t from);

    int64_t GetDistance(size_t to) const override;

    SequencePtr<size_t> GetShortestPath(size_t to) const override;

    PathSteps GetShortestPathWithTransfers(size_t to) const override;

private:
    SequencePtr<AccumulatedPath> dist_;
    SequencePtr<size_t> prev_;
    size_t from_state_;
    size_t vertex_count_;
};
