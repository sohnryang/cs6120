#pragma once

#include "cfg.hh"
#include "pass.hh"

#include "nlohmann/json.hpp"

class DumpControlFlowGraph : public AnalysisPass<ControlFlowGraph> {
private:
  ControlFlowGraph _cfg;

public:
  void operator()(const ControlFlowGraph &cfg) override;

  nlohmann::json serialized_result() override;
};
