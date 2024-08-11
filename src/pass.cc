#include "pass.hh"
#include "cfg.hh"

#include <memory>
#include <vector>

PassRunner::PassRunner(
    const std::vector<std::shared_ptr<OptimizationPass>> &passes)
    : _passes(passes) {}

ControlFlowGraph PassRunner::run_passes(const ControlFlowGraph &cfg) const {
  auto optimized = cfg;
  for (const auto &pass : _passes)
    optimized = (*pass)(optimized);
  return optimized;
}
