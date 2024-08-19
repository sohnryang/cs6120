#pragma once

#include "cfg.hh"
#include "pass.hh"

#include <cstddef>
#include <unordered_set>
#include <vector>

#include "nlohmann/json.hpp"

class DominatorAnalysis : public AnalysisPass<ControlFlowGraph> {
private:
  std::vector<std::unordered_set<std::size_t>> _dominators;

public:
  inline const decltype(_dominators) &dominators() const & {
    return _dominators;
  }

  void operator()(const ControlFlowGraph &cfg) override;

  nlohmann::json serialized_result() override;
};
