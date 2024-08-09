#pragma once

#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "nlohmann/json.hpp"

struct ValueOp;
struct EffectOp;
struct Label;

struct Type {
  std::string name;
  std::shared_ptr<Type> param_type;
};

void to_json(nlohmann::json &j, const Type &type);

void from_json(const nlohmann::json &j, Type &type);

struct Label {
  std::string name;
};

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
};

struct ValueOp {
  ValueOpKind kind;
  std::vector<std::string> args;
  std::vector<std::string> funcs;
  std::vector<std::string> labels;
  std::string dest;
  Type type;
};

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
};

void to_json(nlohmann::json &j, const EffectOp &op);

void from_json(const nlohmann::json &j, EffectOp &op);

struct Constant {
  std::variant<int64_t, bool> value;
  std::string dest;

  inline Type type() const {
    if (value.index() == 0)
      return {.name = "int", .param_type = nullptr};
    else
      return {.name = "bool", .param_type = nullptr};
  };
};

void to_json(nlohmann::json &j, const Constant &c);

void from_json(const nlohmann::json &j, Constant &c);

using Op = std::variant<ValueOp, EffectOp, Constant>;

void to_json(nlohmann::json &j, const Op &op);

void from_json(const nlohmann::json &j, Op &op);

struct Argument {
  std::string name;
  Type type;
};

void to_json(nlohmann::json &j, const Argument &arg);

void from_json(const nlohmann::json &j, Argument &arg);

using Instruction = std::variant<Op, Label>;

void to_json(nlohmann::json &j, const Instruction &instr);

void from_json(const nlohmann::json &j, Instruction &instr);

struct Function {
  std::string name;
  std::vector<Argument> args;
  std::optional<Type> return_type;
  std::vector<Instruction> instrs;
};

void to_json(nlohmann::json &j, const Function &func);

void from_json(const nlohmann::json &j, Function &func);

struct Program {
  std::vector<Function> functions;
};

void to_json(nlohmann::json &j, const Program &prog);

void from_json(const nlohmann::json &j, Program &prog);
