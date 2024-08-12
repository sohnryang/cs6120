#include "cfg.hh"
#include "bril_types.hh"

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "fmt/format.h"

ControlFlowGraph ControlFlowGraph::from_function(const Function &function) {
  std::vector<Op> ops;
  std::vector<BasicBlock> blocks;
  std::optional<std::string> current_block_label;
  std::unordered_map<std::string, std::size_t> label_to_id;
  static const std::unordered_set<EffectOpKind> TERMINATOR_OP_KIND = {
      EffectOpKind::Jmp, EffectOpKind::Br, EffectOpKind::Ret};

  for (const auto &instr : function.instrs) {
    const auto op = try_instr_into_op(instr);
    const auto label = try_instr_into_label(instr);
    const auto block_id = blocks.size();
    const auto name = current_block_label.value_or(
        fmt::format("{}.{}", function.name, block_id));

    std::optional<EffectOp> terminator;
    std::optional<std::string> next_label;
    if (op.has_value()) {
      ops.push_back(*op);
      const auto effect_op = try_op_into_effect_op(*op);
      if (!effect_op.has_value() ||
          !TERMINATOR_OP_KIND.contains(effect_op->kind))
        continue;
      terminator = effect_op;
    } else
      next_label = label->name;

    if (!current_block_label.has_value() && ops.empty()) {
      current_block_label = next_label;
      continue;
    }

    blocks.push_back({
        .name = name,
        .ops = ops,
        .terminator = terminator,
    });
    label_to_id[name] = block_id;
    ops.clear();
    current_block_label = next_label;
  }

  if (current_block_label.has_value() || !ops.empty()) {
    const auto block_id = blocks.size();
    const auto name = current_block_label.value_or(
        fmt::format("{}.{}", function.name, block_id));
    blocks.push_back({
        .name = name,
        .ops = ops,
        .terminator = {},
    });
    label_to_id[name] = block_id;
  }

  std::unordered_map<std::size_t, std::vector<std::size_t>> successors,
      predecessors;
  for (std::size_t i = 0; i < blocks.size(); i++) {
    const auto &terminator = blocks[i].terminator;
    if (terminator.has_value()) {
      switch (terminator->kind) {
      case EffectOpKind::Jmp: {
        const auto dest_id = label_to_id.at(terminator->labels[0]);
        successors[i] = {dest_id};
        predecessors[dest_id].push_back(i);
        break;
      }
      case EffectOpKind::Br: {
        const auto dest_id1 = label_to_id.at(terminator->labels[0]),
                   dest_id2 = label_to_id.at(terminator->labels[1]);
        successors[i] = {dest_id1, dest_id2};
        predecessors[dest_id1].push_back(i);
        predecessors[dest_id2].push_back(i);
        break;
      }
      case EffectOpKind::Ret:
        successors[i] = {};
        break;
      default:
        throw std::runtime_error("");
      }
    } else if (i < blocks.size() - 1) {
      successors[i] = {i + 1};
      predecessors[i + 1].push_back(i);
    } else
      successors[i] = {};
  }

  return {
      .args = function.args,
      .blocks = blocks,
      .successors = successors,
      .predecessors = predecessors,
      .label_to_id = label_to_id,
  };
}

std::vector<Instruction> ControlFlowGraph::into_instrs() const {
  std::vector<Instruction> instrs;
  for (const auto &block : blocks) {
    instrs.push_back(Label{.name = block.name});
    for (const auto &op : block.ops)
      instrs.push_back(op);
  }
  return instrs;
}
