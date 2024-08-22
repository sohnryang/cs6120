#include "bril_types.hh"
#include "cfg.hh"
#include "dom.hh"
#include "dump_cfg.hh"
#include "local_dce.hh"
#include "natural_loop.hh"
#include "pass.hh"
#include "reaching_def.hh"

#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "argparse/argparse.hpp"
#include "fmt/format.h"
#include "fmt/ostream.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

static const std::unordered_map<std::string, Pass<ControlFlowGraph>>
    CFG_PASSES = {
        {"local_dce", std::make_shared<LocalDeadCodeElimination>()},
        {"dump_cfg", std::make_shared<DumpControlFlowGraph>()},
        {"dom", std::make_shared<DominatorAnalysis>()},
        {"natural_loop", std::make_shared<NaturalLoopAnalysis>()},
        {"reaching_def", std::make_shared<ReachingDefinitionAnalysis>()},
};

int main(int argc, char *argv[]) {
  argparse::ArgumentParser parser("opt");
  std::vector<std::string> cfg_pass_names;
  std::string analysis_out_filename;
  parser.add_argument("--cfg-pass").append().store_into(cfg_pass_names);
  parser.add_argument("--analysis-out")
      .default_value("analysis_out.json")
      .store_into(analysis_out_filename);

  try {
    parser.parse_args(argc, argv);
  } catch (const std::exception &err) {
    fmt::println(std::cerr, "Failed to parse args: {}", err.what());
    std::exit(1);
  }

  std::vector<Pass<ControlFlowGraph>> cfg_passes;
  for (const auto &pass_name : cfg_pass_names)
    cfg_passes.push_back(CFG_PASSES.at(pass_name));
  PassRunner<ControlFlowGraph> cfg_pass_runner(cfg_passes);

  json j;
  std::cin >> j;
  Program program(j);

  Program optimized_program;
  json analysis_out;
  for (const auto &function : program.functions) {
    const auto [optimized, serialized_results] =
        cfg_pass_runner(ControlFlowGraph::from_function(function));
    optimized_program.functions.push_back({
        .name = function.name,
        .args = function.args,
        .return_type = function.return_type,
        .instrs = optimized.into_instrs(),
    });
    analysis_out["cfg"][function.name] = serialized_results;
  }

  json serialized = optimized_program;
  fmt::println(std::cout, "{}", fmt::streamed(serialized));

  std::ofstream outfile(analysis_out_filename);
  outfile << analysis_out;
}
