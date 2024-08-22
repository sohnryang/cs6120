#pragma once

#include "cfg.hh"

#include <cstddef>
#include <queue>
#include <vector>

template <class T> class DataflowSolver {
private:
  std::vector<T> _in_states;

  std::vector<T> _out_states;

public:
  inline const decltype(_in_states) &in_states() const & { return _in_states; }

  inline const decltype(_out_states) &out_states() const & {
    return _out_states;
  }

  static T initial_state(const ControlFlowGraph &cfg, std::size_t block_id);

  static T join(const T &lhs, const T &rhs);

  static T transfer(const ControlFlowGraph &cfg, std::size_t block_id,
                    const T &in_state);

  void operator()(const ControlFlowGraph &cfg);
};

template <class T>
void DataflowSolver<T>::operator()(const ControlFlowGraph &cfg) {
  _in_states = {};
  _out_states = {};
  for (std::size_t i = 0; i < cfg.blocks.size(); i++) {
    const auto init = initial_state(cfg, i);
    _in_states.push_back(init);
    _out_states.push_back(init);
  }

  std::queue<std::size_t> worklist;
  for (std::size_t i = 0; i < cfg.blocks.size(); i++)
    worklist.push(i);
  while (!worklist.empty()) {
    const auto block_id = worklist.front();
    worklist.pop();

    _in_states[block_id] = {};
    for (const auto pred : cfg.graph.predecessors[block_id])
      _in_states[block_id] = join(_in_states[block_id], _out_states[pred]);
    const auto new_out_state = transfer(cfg, block_id, _in_states[block_id]);
    if (new_out_state == _out_states[block_id])
      continue;

    _out_states[block_id] = new_out_state;
    for (const auto succ : cfg.graph.successors[block_id])
      worklist.push(succ);
  }
}
