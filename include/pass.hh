#pragma once

#include "utils.hh"

#include <memory>
#include <utility>
#include <variant>
#include <vector>

#include "nlohmann/json.hpp"

template <class T> class TransformPass {
public:
  virtual T operator()(const T &) const = 0;
};

template <class T> class AnalysisPass {
public:
  virtual void operator()(const T &) = 0;

  virtual nlohmann::json serialized_result() = 0;
};

template <class T>
using Pass = std::variant<std::shared_ptr<TransformPass<T>>,
                          std::shared_ptr<AnalysisPass<T>>>;

template <class T> class PassRunner {
private:
  std::vector<Pass<T>> _passes;

public:
  PassRunner() = default;
  PassRunner(const PassRunner &) = default;
  PassRunner(PassRunner &&) = default;
  PassRunner &operator=(const PassRunner &) = default;
  PassRunner &operator=(PassRunner &&) = default;

  PassRunner(const std::vector<Pass<T>> &passes);

  std::pair<T, nlohmann::json> operator()(const T &pass_input);
};

template <class T>
PassRunner<T>::PassRunner(const std::vector<Pass<T>> &passes)
    : _passes(passes) {}

template <class T>
std::pair<T, nlohmann::json> PassRunner<T>::operator()(const T &pass_input) {
  std::vector<nlohmann::json> results;
  auto transformed = pass_input;
  for (const auto pass : _passes)
    std::visit(match{
                   [&](std::shared_ptr<TransformPass<T>> transform_pass) {
                     transformed = (*transform_pass)(transformed);
                   },
                   [&](std::shared_ptr<AnalysisPass<T>> analysis_pass) {
                     (*analysis_pass)(transformed);
                     results.push_back(analysis_pass->serialized_result());
                   },
               },
               pass);
  return {transformed, results};
}
