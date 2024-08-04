#pragma once

#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

struct ValueOp;
struct EffectOp;
struct Label;

struct Type {
  std::string name;
  std::shared_ptr<Type> param_type;
};

struct Label {
  std::string name;
};

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

enum class EffectOpKind {
  Jmp,
  Br,
  Call,
  Ret,
};

struct EffectOp {
  EffectOpKind kind;
  std::vector<std::string> args;
  std::vector<std::string> funcs;
  std::vector<std::string> labels;
};

struct Constant {
  std::variant<int64_t, bool> value;

  inline Type type() {
    if (value.index() == 0)
      return {.name = "int", .param_type = nullptr};
    else
      return {.name = "bool", .param_type = nullptr};
  };
};

using Op = std::variant<ValueOp, EffectOp, Constant>;

struct Argument {
  std::string name;
  Type type;
};

using Instruction = std::variant<Op, Label>;

struct Function {
  std::string name;
  std::vector<Argument> args;
  std::optional<Type> return_type;
  std::vector<Instruction> instrs;
};

struct Program {
  std::vector<Function> functions;
};
