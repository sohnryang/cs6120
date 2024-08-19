#include "graph.hh"
#include "utils.hh"

#include <algorithm>
#include <cstddef>
#include <functional>
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

template <> struct std::hash<std::vector<std::size_t>> {
  std::size_t operator()(const std::vector<std::size_t> &vec) const {
    std::size_t res = vec.size();
    for (const auto elem : vec)
      hash_combine(res, elem);
    return res;
  }
};

template <> struct std::hash<SparseAdjacencyList> {
  std::size_t operator()(const SparseAdjacencyList &sparse_adj_list) const {
    std::vector<std::size_t> keys;
    for (const auto &p : sparse_adj_list)
      keys.push_back(p.first);
    std::sort(keys.begin(), keys.end());

    std::size_t res = keys.size();
    for (const auto key : keys)
      hash_combine(res, sparse_adj_list.at(key));
    return res;
  }
};

MAKE_HASHABLE_IMPL(SparseGraph, t.successors, t.predecessors);
