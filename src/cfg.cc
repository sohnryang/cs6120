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
  std::vector<Op> instrs;
  std::vector<BasicBlock> blocks;
  std::optional<std::string> current_block_label;
  std::unordered_map<std::string, std::size_t> label_to_id;
  std::unordered_map<std::size_t, std::vector<std::size_t>> successors;
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
      instrs.push_back(*op);
      const auto effect_op = try_op_into_effect_op(*op);
      if (!effect_op.has_value() ||
          !TERMINATOR_OP_KIND.contains(effect_op->kind))
        continue;
      terminator = effect_op;
    } else
      next_label = label->name;

    if (!current_block_label.has_value() && instrs.empty()) {
      current_block_label = next_label;
      continue;
    }

    blocks.push_back({
        .name = name,
        .instrs = instrs,
        .terminator = terminator,
    });
    label_to_id[name] = block_id;
    instrs.clear();
    current_block_label = next_label;
  }

  if (current_block_label.has_value() || !instrs.empty()) {
    const auto block_id = blocks.size();
    const auto name = current_block_label.value_or(
        fmt::format("{}.{}", function.name, block_id));
    blocks.push_back({
        .name = name,
        .instrs = instrs,
        .terminator = {},
    });
    label_to_id[name] = block_id;
  }

  for (std::size_t i = 0; i < blocks.size(); i++) {
    const auto &terminator = blocks[i].terminator;
    if (terminator.has_value()) {
      switch (terminator->kind) {
      case EffectOpKind::Jmp:
        successors[i] = {label_to_id.at(terminator->labels[0])};
        break;
      case EffectOpKind::Br:
        successors[i] = {
            label_to_id.at(terminator->labels[0]),
            label_to_id.at(terminator->labels[1]),
        };
        break;
      case EffectOpKind::Ret:
        successors[i] = {};
        break;
      default:
        throw std::runtime_error("");
      }
    } else if (i == blocks.size() - 1)
      successors[i] = {};
    else
      successors[i] = {i + 1};
  }

  return {.blocks = blocks, .successors = successors};
}
