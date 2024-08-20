#include "natural_loop.hh"
#include "cfg.hh"
#include "dom.hh"
#include "graph.hh"

#include <cstddef>
#include <unordered_set>
#include <vector>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

void NaturalLoopAnalysis::dfs(
    const DenseGraph &graph,
    const std::vector<std::unordered_set<std::size_t>> &dominators,
    std::size_t here) {
  _visited[here] = true;
  for (const auto there : graph.successors[here]) {
    if (dominators[here].contains(there))
      _back_edges[here].insert(there);
    if (_visited[there])
      continue;

    dfs(graph, dominators, there);
  }
}

void NaturalLoopAnalysis::collect_back_edges(
    const ControlFlowGraph &cfg,
    const std::vector<std::unordered_set<std::size_t>> &dominators) {
  _back_edges.assign(cfg.blocks.size(), {});
  _visited.assign(cfg.blocks.size(), false);
  dfs(cfg.graph, dominators, 0);
}

void NaturalLoopAnalysis::operator()(const ControlFlowGraph &cfg) {
  DominatorAnalysis dom;
  dom(cfg);
  const auto dominators = dom.dominators();
  collect_back_edges(cfg, dominators);

  for (std::size_t src = 0; src < _back_edges.size(); src++) {
    for (const auto dest : _back_edges[src]) {
      std::unordered_set<std::size_t> loop_vertices;
      loop_vertices.insert(dest);
      std::vector<std::size_t> stack = {src};
      while (!stack.empty()) {
        const auto here = stack.back();
        stack.pop_back();
        if (loop_vertices.contains(here))
          continue;

        loop_vertices.insert(here);
        for (const auto p : cfg.graph.predecessors[here])
          stack.push_back(p);
      }

      const auto loop_subgraph =
          SparseGraph::from_dense(cfg.graph, loop_vertices);
      _natural_loops.insert(loop_subgraph);
    }
  }
}

json NaturalLoopAnalysis::serialized_result() {
  json j;
  j["back_edges"] = _back_edges;
  j["natural_loops"] = _natural_loops;
  return j;
}
