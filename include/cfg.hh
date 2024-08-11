#pragma once

#include "bril_types.hh"

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

struct BasicBlock {
  std::string name;
  std::vector<Op> ops;
  std::optional<EffectOp> terminator;

  bool operator==(const BasicBlock &) const = default;
};

struct ControlFlowGraph {
  std::vector<Argument> args;
  std::vector<BasicBlock> blocks;
  std::unordered_map<std::size_t, std::vector<std::size_t>> successors;

  bool operator==(const ControlFlowGraph &) const = default;

  static ControlFlowGraph from_function(const Function &function);

  std::vector<Instruction> into_instrs() const;
};
