#pragma once

#include "bril_types.hh"

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

struct BasicBlock {
  std::string name;
  std::vector<Op> instrs;
  std::optional<EffectOp> terminator;
};

struct ControlFlowGraph {
  std::vector<Argument> args;
  std::vector<BasicBlock> blocks;
  std::unordered_map<std::size_t, std::vector<std::size_t>> successors;

  static ControlFlowGraph from_function(const Function &function);

  std::vector<Instruction> into_instrs() const;
};
