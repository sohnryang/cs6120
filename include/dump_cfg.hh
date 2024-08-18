#pragma once

#include "cfg.hh"
#include "pass.hh"

#include "nlohmann/json.hpp"

class DumpControlFlowGraph : public AnalysisPass<ControlFlowGraph> {
private:
  ControlFlowGraph _cfg;

public:
  DumpControlFlowGraph() = default;
  DumpControlFlowGraph(const DumpControlFlowGraph &) = default;
  DumpControlFlowGraph(DumpControlFlowGraph &&) = default;
  DumpControlFlowGraph &operator=(const DumpControlFlowGraph &) = default;
  DumpControlFlowGraph &operator=(DumpControlFlowGraph &&) = default;

  void operator()(const ControlFlowGraph &cfg) override;

  nlohmann::json serialized_result() override;
};
