#include "reaching_def.hh"
#include "bril_types.hh"
#include "cfg.hh"
#include "dataflow.hh"
#include "utils.hh"

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>

#include "nlohmann/json_fwd.hpp"

using json = nlohmann::json;

void to_json(json &j, const ArgumentDefinition &argument_definition) {
  j["arg_index"] = argument_definition.index;
}

void to_json(json &j, const LocalDefinition &local_definition) {
  j["block_id"] = local_definition.block_id;
  j["op_index"] = local_definition.op_index;
}

void to_json(json &j, const Definition &definition) {
  std::visit([&](auto &&arg) { to_json(j, arg); }, definition);
}

template <>
ReachingDefinitions
DataflowSolver<ReachingDefinitions>::initial_state(const ControlFlowGraph &cfg,
                                                   std::size_t block_id) {
  if (block_id != 0)
    return {};

  ReachingDefinitions res;
  for (std::size_t i = 0; i < cfg.args.size(); i++)
    res[cfg.args[i].name] = {ArgumentDefinition{.index = i}};
  return res;
}

template <>
ReachingDefinitions
DataflowSolver<ReachingDefinitions>::join(const ReachingDefinitions &lhs,
                                          const ReachingDefinitions &rhs) {
  ReachingDefinitions res = lhs;
  for (const auto &p : rhs)
    for (const auto &location : p.second)
      res[p.first].insert(location);
  return res;
}

template <>
ReachingDefinitions DataflowSolver<ReachingDefinitions>::transfer(
    const ControlFlowGraph &cfg, std::size_t block_id,
    const ReachingDefinitions &in_state) {
  std::unordered_map<std::string, LocalDefinition> gen;
  std::unordered_set<std::string> kill;
  const auto &basic_block = cfg.blocks[block_id];
  for (std::size_t i = 0; i < basic_block.ops.size(); i++) {
    const auto &op = basic_block.ops[i];
    std::optional<std::string> destination;
    std::visit(
        match{
            [&](const Constant &constant) { destination = constant.dest; },
            [&](const ValueOp &value_op) { destination = value_op.dest; },
            [&](auto) {},
        },
        op);
    if (!destination.has_value())
      continue;

    if (in_state.contains(*destination))
      kill.insert(*destination);
    gen[*destination] = {.block_id = block_id, .op_index = i};
  }

  ReachingDefinitions out = in_state;
  for (const auto &killed_name : kill)
    out.erase(killed_name);
  for (const auto &p : gen)
    out[p.first].insert(p.second);
  return out;
}

void ReachingDefinitionAnalysis::operator()(const ControlFlowGraph &cfg) {
  DataflowSolver<ReachingDefinitions> solver;
  solver(cfg);
  _reaching_definitions = solver.in_states();
}

json ReachingDefinitionAnalysis::serialized_result() {
  return _reaching_definitions;
}
