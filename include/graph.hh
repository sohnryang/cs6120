#pragma once

#include <cstddef>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "nlohmann/json.hpp"

using DenseAdjacencyList = std::vector<std::vector<std::size_t>>;

struct DenseGraph {
  DenseAdjacencyList successors;
  DenseAdjacencyList predecessors;

  bool operator==(const DenseGraph &) const = default;
};

void to_json(nlohmann::json &j, const DenseGraph &dense_graph);

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

void to_json(nlohmann::json &j, const SparseGraph &sparse_graph);

template <> struct std::hash<SparseGraph> {
  std::size_t operator()(const SparseGraph &sparse_graph) const;
};
