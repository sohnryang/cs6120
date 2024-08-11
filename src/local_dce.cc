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

ControlFlowGraph
LocalDeadCodeElimination::operator()(const ControlFlowGraph &cfg) const {
  std::vector<BasicBlock> blocks;

  for (const auto &block : cfg.blocks) {
    std::vector<DefinitionState> definition_states(block.instrs.size(),
                                                   DefinitionState::Unused);
    std::unordered_map<std::string, std::size_t> definition_locations;
    for (std::size_t i = 0; i < block.instrs.size(); i++) {
      const auto &instr = block.instrs[i];

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
                 instr);

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

    std::vector<Op> instrs;
    for (std::size_t i = 0; i < block.instrs.size(); i++) {
      const auto &instr = block.instrs[i];

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
          instr);
      if (is_dead)
        continue;

      instrs.push_back(instr);
    }

    blocks.push_back({
        .name = block.name,
        .instrs = instrs,
        .terminator = block.terminator,
    });
  }
  return {.blocks = blocks, .successors = cfg.successors};
}
