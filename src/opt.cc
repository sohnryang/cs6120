#include "bril_types.hh"
#include "cfg.hh"
#include "dead_code.hh"
#include "pass.hh"

#include <cstdlib>
#include <exception>
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

static const std::unordered_map<std::string, std::shared_ptr<OptimizationPass>>
    PASSES = {
        {"dead_code_elimination", std::make_shared<DeadCodeElimination>()},
};

int main(int argc, char *argv[]) {
  argparse::ArgumentParser parser("opt");
  std::vector<std::string> pass_names;
  parser.add_argument("--pass").append().store_into(pass_names);

  try {
    parser.parse_args(argc, argv);
  } catch (const std::exception &err) {
    fmt::println(std::cerr, "Failed to parse args: {}", err.what());
    std::exit(1);
  }

  std::vector<std::shared_ptr<OptimizationPass>> passes;
  for (const auto &pass_name : pass_names)
    passes.push_back(PASSES.at(pass_name));
  PassRunner runner(passes);

  json j;
  std::cin >> j;
  Program program(j);

  Program optimized_program;
  for (const auto &function : program.functions) {
    const auto optimized =
        runner.run_passes(ControlFlowGraph::from_function(function));
    optimized_program.functions.push_back({
        .name = function.name,
        .args = function.args,
        .return_type = function.return_type,
        .instrs = optimized.into_instrs(),
    });
  }

  json serialized = optimized_program;
  fmt::println(std::cout, "{}", fmt::streamed(serialized));
}
