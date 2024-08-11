#pragma once

#include "cfg.hh"
#include "pass.hh"

class DeadCodeElimination : public OptimizationPass {
public:
  virtual ControlFlowGraph
  operator()(const ControlFlowGraph &cfg) const override;
};
