#pragma once

#include "bril_types.hh"
#include "graph.hh"

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
  DenseGraph graph;
  std::unordered_map<std::string, std::size_t> label_to_id;

  bool operator==(const ControlFlowGraph &) const = default;

  static ControlFlowGraph from_function(const Function &function);

  std::vector<Instruction> into_instrs() const;
};
