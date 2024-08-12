#pragma once

#include "bril_types.hh"

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
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
  std::vector<std::vector<std::size_t>> successors;
  std::vector<std::vector<std::size_t>> predecessors;
  std::unordered_map<std::string, std::size_t> label_to_id;

  bool operator==(const ControlFlowGraph &) const = default;

  static ControlFlowGraph from_function(const Function &function);

  std::vector<Instruction> into_instrs() const;

  std::vector<std::unordered_set<std::size_t>> dominators() const;
};
