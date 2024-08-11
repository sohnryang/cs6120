#include "local_dce.hh"
#include "bril_types.hh"
#include "cfg.hh"
#include "utils.hh"

#include <cstddef>
#include <functional>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <variant>
#include <vector>

#include "fmt/format.h"

enum class DefinitionState {
  Unused,
  Used,
  Overwritten,
};

ControlFlowGraph optimize_single_step(const ControlFlowGraph &cfg) {
  std::vector<BasicBlock> blocks;

  for (const auto &block : cfg.blocks) {
    std::vector<DefinitionState> definition_states(block.ops.size(),
                                                   DefinitionState::Unused);
    std::unordered_map<std::string, std::size_t> definition_locations;
    for (std::size_t i = 0; i < block.ops.size(); i++) {
      const auto &op = block.ops[i];

      std::optional<std::reference_wrapper<const std::vector<std::string>>>
          args;
      std::optional<std::string> dest;
      std::visit(match{
                     [&](const ValueOp &value_op) {
                       args = std::optional{std::ref(value_op.args)};
                       dest = value_op.dest;
                     },
                     [&](const EffectOp &effect_op) {
                       args = std::optional{std::ref(effect_op.args)};
                     },
                     [&](const Constant &constant) { dest = constant.dest; },
                 },
                 op);

      if (args.has_value())
        for (const auto &arg : (*args).get()) {
          if (!definition_locations.contains(arg))
            continue;

          const auto location = definition_locations.at(arg);
          if (definition_states[location] == DefinitionState::Overwritten)
            throw std::runtime_error(fmt::format(
                "Reference to overwritten definition at {} of block {}",
                location, block.name));
          definition_states[location] = DefinitionState::Used;
        }

      if (dest.has_value()) {
        const auto prev_location_it = definition_locations.find(*dest);
        if (prev_location_it != definition_locations.end()) {
          const auto prev_location = prev_location_it->second;
          switch (definition_states[prev_location]) {
          case DefinitionState::Unused:
            definition_states[prev_location] = DefinitionState::Overwritten;
            break;
          case DefinitionState::Used:
            break;
          case DefinitionState::Overwritten:
            throw std::runtime_error(fmt::format(
                "Reference to overwritten definition at {} of block {}",
                prev_location, block.name));
          }
        }
        definition_locations[*dest] = i;
      }
    }

    std::vector<Op> ops;
    for (std::size_t i = 0; i < block.ops.size(); i++) {
      const auto &op = block.ops[i];

      const auto is_dead = std::visit(
          match{
              [&](const ValueOp &value_op) {
                if (value_op.kind == ValueOpKind::Call)
                  return false;
                return definition_states[i] == DefinitionState::Overwritten;
              },
              [&](const EffectOp &effect_op) { return false; },
              [&](const Constant &constant) {
                return definition_states[i] == DefinitionState::Overwritten;
              },
          },
          op);
      if (is_dead)
        continue;

      ops.push_back(op);
    }

    blocks.push_back({
        .name = block.name,
        .ops = ops,
        .terminator = block.terminator,
    });
  }
  return {.blocks = blocks, .successors = cfg.successors};
}

ControlFlowGraph
LocalDeadCodeElimination::operator()(const ControlFlowGraph &cfg) const {
  auto last = cfg;
  while (true) {
    const auto optimized = optimize_single_step(last);
    if (optimized == last)
      return optimized;
    last = optimized;
  }
}
