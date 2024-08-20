#pragma once

#include "cfg.hh"
#include "graph.hh"
#include "pass.hh"

#include <cstddef>
#include <unordered_set>
#include <vector>

#include "nlohmann/json.hpp"

class NaturalLoopAnalysis : public AnalysisPass<ControlFlowGraph> {
private:
  std::unordered_set<SparseGraph> _natural_loops;
  std::vector<std::unordered_set<std::size_t>> _back_edges;
  std::vector<bool> _visited;

public:
  inline const decltype(_natural_loops) &natural_loops() const & {
    return _natural_loops;
  }

  inline const decltype(_back_edges) &back_edges() const & {
    return _back_edges;
  }

  void dfs(const DenseGraph &graph,
           const std::vector<std::unordered_set<std::size_t>> &dominators,
           std::size_t here);

  void collect_back_edges(
      const ControlFlowGraph &cfg,
      const std::vector<std::unordered_set<std::size_t>> &dominators);

  void operator()(const ControlFlowGraph &cfg) override;

  nlohmann::json serialized_result() override;
};
