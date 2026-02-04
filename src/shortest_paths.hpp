#pragma once

#include "ishortest_paths.hpp"

class Dijkstra : public IShortestPathsFinder {
public:
    Dijkstra(IGraphPtr graph, size_t from);

    int64_t GetDistance(size_t to) const override;

    SequencePtr<size_t> GetShortestPath(size_t to) const override;

private:
    SequencePtr<int64_t> dist_;
    SequencePtr<size_t> prev_;
    size_t from_;
};

class FordBellman : public IShortestPathsFinder {
public:
    FordBellman(IGraphPtr graph, size_t from);

    int64_t GetDistance(size_t to) const override;

    SequencePtr<size_t> GetShortestPath(size_t to) const override;

private:
    SequencePtr<int64_t> dist_;
    SequencePtr<size_t> prev_;
    size_t from_;
};
