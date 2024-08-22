#pragma once

#include "cfg.hh"
#include "pass.hh"
#include "utils.hh"

#include <cstddef>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include "nlohmann/json.hpp"

struct ArgumentDefinition {
  std::size_t index;

  bool operator==(const ArgumentDefinition &) const = default;
};

MAKE_HASHABLE(ArgumentDefinition, t.index);

void to_json(nlohmann::json &j, const ArgumentDefinition &argument_definition);

struct LocalDefinition {
  std::size_t block_id;
  std::size_t op_index;

  bool operator==(const LocalDefinition &) const = default;
};

MAKE_HASHABLE(LocalDefinition, t.block_id, t.op_index);

void to_json(nlohmann::json &j, const LocalDefinition &local_definition);

using Definition = std::variant<ArgumentDefinition, LocalDefinition>;

void to_json(nlohmann::json &j, const Definition &definition);

using ReachingDefinitions =
    std::unordered_map<std::string, std::unordered_set<Definition>>;

class ReachingDefinitionAnalysis : public AnalysisPass<ControlFlowGraph> {
private:
  std::vector<ReachingDefinitions> _reaching_definitions;

public:
  inline const decltype(_reaching_definitions) &reaching_definitions() const & {
    return _reaching_definitions;
  }

  void operator()(const ControlFlowGraph &cfg) override;

  nlohmann::json serialized_result() override;
};
