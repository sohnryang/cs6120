#pragma once

#include <memory>
#include <vector>

template <class T> class TransformPass {
public:
  virtual T operator()(const T &) const = 0;
};

template <class T> class TransformPassRunner {
private:
  std::vector<std::shared_ptr<TransformPass<T>>> _passes;

public:
  TransformPassRunner() = default;
  TransformPassRunner(const TransformPassRunner &) = default;
  TransformPassRunner(TransformPassRunner &&) = default;
  TransformPassRunner &operator=(const TransformPassRunner &) = default;
  TransformPassRunner &operator=(TransformPassRunner &&) = default;

  TransformPassRunner(
      const std::vector<std::shared_ptr<TransformPass<T>>> &passes);

  T run_passes(const T &cfg) const;
};

template <class T>
TransformPassRunner<T>::TransformPassRunner(
    const std::vector<std::shared_ptr<TransformPass<T>>> &passes)
    : _passes(passes) {}

template <class T>
T TransformPassRunner<T>::run_passes(const T &pass_input) const {
  auto transformed = pass_input;
  for (const auto &pass : _passes)
    transformed = (*pass)(transformed);
  return transformed;
}
