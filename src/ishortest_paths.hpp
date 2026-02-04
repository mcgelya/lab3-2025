#pragma once

#include "fwd.hpp"

class IShortestPathsFinder {
public:
    virtual ~IShortestPathsFinder() = default;

    virtual int64_t GetDistance(size_t to) const = 0;

    virtual SequencePtr<size_t> GetShortestPath(size_t to) const = 0;
};
