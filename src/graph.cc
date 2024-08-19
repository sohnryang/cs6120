#include "graph.hh"

#include <cstddef>
#include <unordered_set>
#include <vector>

SparseGraph
SparseGraph::from_dense(const DenseGraph &dense_graph,
                        const std::unordered_set<std::size_t> &vertices) {
  SparseGraph res;
  for (const auto v : vertices) {
    res.successors[v] = dense_graph.successors[v];
    res.predecessors[v] = dense_graph.predecessors[v];
    std::erase_if(res.successors[v],
                  [&](const auto w) { return !vertices.contains(w); });
    std::erase_if(res.predecessors[v],
                  [&](const auto w) { return !vertices.contains(w); });
  }
  return res;
}
