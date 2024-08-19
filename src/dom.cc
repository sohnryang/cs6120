#include "dom.hh"
#include "cfg.hh"

#include <cstddef>
#include <unordered_set>

#include "nlohmann/json.hpp"

// TODO: use more efficient algorithm
void DominatorAnalysis::operator()(const ControlFlowGraph &cfg) {
  _dominators.assign(cfg.blocks.size(), {});
  std::unordered_set<std::size_t> universe;
  for (std::size_t i = 0; i < cfg.blocks.size(); i++)
    universe.insert(i);
  _dominators[0] = {0};
  for (std::size_t v = 1; v < cfg.blocks.size(); v++)
    _dominators[v] = universe;

  while (true) {
    auto updated = _dominators;
    for (std::size_t v = 1; v < cfg.blocks.size(); v++) {
      auto updated_set = universe;
      for (const auto p : cfg.graph.predecessors[v])
        std::erase_if(updated_set,
                      [&](const auto u) { return !updated[p].contains(u); });
      updated_set.insert(v);
      updated[v] = updated_set;
    }

    if (updated == _dominators)
      return;
    _dominators = updated;
  }
}

nlohmann::json DominatorAnalysis::serialized_result() { return _dominators; }
