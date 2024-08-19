#include "dump_cfg.hh"
#include "cfg.hh"

#include "nlohmann/json.hpp"

using json = nlohmann::json;

void to_json(json &j, const BasicBlock &basic_block) {
  j["name"] = basic_block.name;
  j["ops"] = basic_block.ops;
  if (basic_block.terminator.has_value())
    j["terminator"] = *basic_block.terminator;
}

void to_json(json &j, const ControlFlowGraph &cfg) {
  j["args"] = cfg.args;
  j["blocks"] = cfg.blocks;
  j["graph"] = cfg.graph;
  j["label_to_id"] = cfg.label_to_id;
}

void DumpControlFlowGraph::operator()(const ControlFlowGraph &cfg) {
  _cfg = cfg;
}

json DumpControlFlowGraph::serialized_result() { return _cfg; }
