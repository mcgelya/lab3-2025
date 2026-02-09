#pragma once

#include <memory>

template <typename T>
class Sequence;

template <typename T>
using SequencePtr = std::shared_ptr<Sequence<T>>;

struct Vertex;

using VertexPtr = std::shared_ptr<Vertex>;

class IGraph;

using IGraphPtr = std::shared_ptr<IGraph>;

class IShortestPathsFinder;

using IShortestPathsFinderPtr = std::shared_ptr<IShortestPathsFinder>;
