#pragma once

#include "utils.hh"

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "nlohmann/json.hpp"

struct ValueOp;
struct EffectOp;
struct Label;

template <> struct std::hash<std::vector<std::string>> {
  std::size_t operator()(const std::vector<std::string> &vec) const;
};

struct Type {
  std::string name;
  std::shared_ptr<Type> param_type;

  bool operator==(const Type &other) const;
};

template <> struct std::hash<Type> {
  std::size_t operator()(const Type &type) const;
};

void to_json(nlohmann::json &j, const Type &type);

void from_json(const nlohmann::json &j, Type &type);

struct Label {
  std::string name;

  bool operator==(const Label &) const = default;
};

MAKE_HASHABLE(Label, t.name)

void to_json(nlohmann::json &j, const Label &l);

void from_json(const nlohmann::json &j, Label &l);

enum class ValueOpKind {
  Add,
  Mul,
  Sub,
  Div,
  Eq,
  Lt,
  Gt,
  Le,
  Ge,
  Not,
  And,
  Or,
  Call,
  Id,
};

struct ValueOp {
  ValueOpKind kind;
  std::vector<std::string> args;
  std::vector<std::string> funcs;
  std::vector<std::string> labels;
  std::string dest;
  Type type;

  bool operator==(const ValueOp &) const = default;
};

MAKE_HASHABLE(ValueOp, t.kind, t.args, t.funcs, t.labels, t.dest, t.type);

void to_json(nlohmann::json &j, const ValueOp &op);

void from_json(const nlohmann::json &j, ValueOp &op);

enum class EffectOpKind {
  Jmp,
  Br,
  Call,
  Ret,
  Print,
};

struct EffectOp {
  EffectOpKind kind;
  std::vector<std::string> args;
  std::vector<std::string> funcs;
  std::vector<std::string> labels;

  bool operator==(const EffectOp &) const = default;
};

MAKE_HASHABLE(EffectOp, t.kind, t.args, t.funcs, t.labels)

void to_json(nlohmann::json &j, const EffectOp &op);

void from_json(const nlohmann::json &j, EffectOp &op);

struct Constant {
  std::variant<int64_t, bool> value;
  std::string dest;

  bool operator==(const Constant &) const = default;

  inline Type type() const {
    if (value.index() == 0)
      return {.name = "int", .param_type = nullptr};
    else
      return {.name = "bool", .param_type = nullptr};
  };
};

MAKE_HASHABLE(Constant, t.value, t.dest);

void to_json(nlohmann::json &j, const Constant &c);

void from_json(const nlohmann::json &j, Constant &c);

using Op = std::variant<ValueOp, EffectOp, Constant>;

std::optional<ValueOp> try_op_into_value_op(const Op &op);

std::optional<EffectOp> try_op_into_effect_op(const Op &op);

std::optional<Constant> try_op_into_constant(const Op &op);

void to_json(nlohmann::json &j, const Op &op);

void from_json(const nlohmann::json &j, Op &op);

struct Argument {
  std::string name;
  Type type;

  bool operator==(const Argument &) const = default;
};

void to_json(nlohmann::json &j, const Argument &arg);

void from_json(const nlohmann::json &j, Argument &arg);

using Instruction = std::variant<Op, Label>;

std::optional<Op> try_instr_into_op(const Instruction &instr);

std::optional<Label> try_instr_into_label(const Instruction &instr);

void to_json(nlohmann::json &j, const Instruction &instr);

void from_json(const nlohmann::json &j, Instruction &instr);

struct Function {
  std::string name;
  std::vector<Argument> args;
  std::optional<Type> return_type;
  std::vector<Instruction> instrs;

  bool operator==(const Function &) const = default;
};

void to_json(nlohmann::json &j, const Function &func);

void from_json(const nlohmann::json &j, Function &func);

struct Program {
  std::vector<Function> functions;

  bool operator==(const Program &) const = default;
};

void to_json(nlohmann::json &j, const Program &prog);

void from_json(const nlohmann::json &j, Program &prog);
