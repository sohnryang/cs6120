#pragma once

#include <cstddef>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using DenseAdjacencyList = std::vector<std::vector<std::size_t>>;

struct DenseGraph {
  DenseAdjacencyList successors;
  DenseAdjacencyList predecessors;

  bool operator==(const DenseGraph &) const = default;
};

using SparseAdjacencyList =
    std::unordered_map<std::size_t, std::vector<std::size_t>>;

struct SparseGraph {
  SparseAdjacencyList successors;
  SparseAdjacencyList predecessors;

  bool operator==(const SparseGraph &) const = default;

  static SparseGraph
  from_dense(const DenseGraph &dense_graph,
             const std::unordered_set<std::size_t> &vertices);
};

template <> struct std::hash<SparseGraph> {
  std::size_t operator()(const SparseGraph &sparse_graph) const;
};
