#pragma once

#include <cstdint>

#include "igraph.hpp"

struct PathStep {
    size_t vertex = 0;
    Transport transport = Transport::Feet;
    bool is_transfer = false;
};

using PathSteps = SequencePtr<PathStep>;

class IShortestPathsFinder {
public:
    virtual ~IShortestPathsFinder() = default;

    virtual int64_t GetDistance(size_t to) const = 0;

    virtual SequencePtr<size_t> GetShortestPath(size_t to) const = 0;

    virtual PathSteps GetShortestPathWithTransfers(size_t to) const = 0;
};
