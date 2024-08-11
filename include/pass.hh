#pragma once

#include "cfg.hh"

#include <memory>
#include <vector>

class OptimizationPass {
public:
  virtual ControlFlowGraph operator()(const ControlFlowGraph &) const = 0;
};

class PassRunner {
private:
  std::vector<std::shared_ptr<OptimizationPass>> _passes;

public:
  PassRunner() = default;
  PassRunner(const PassRunner &) = default;
  PassRunner(PassRunner &&) = default;
  PassRunner &operator=(const PassRunner &) = default;
  PassRunner &operator=(PassRunner &&) = default;

  PassRunner(const std::vector<std::shared_ptr<OptimizationPass>> &passes);

  ControlFlowGraph run_passes(const ControlFlowGraph &cfg) const;
};
