#pragma once

#include "cfg.hh"
#include "pass.hh"

class LocalDeadCodeElimination : public OptimizationPass {
public:
  virtual ControlFlowGraph
  operator()(const ControlFlowGraph &cfg) const override;
};
