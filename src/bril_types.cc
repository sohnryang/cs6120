#include "bril_types.hh"
#include "utils.hh"

#include <optional>
#include <variant>

#include "fmt/format.h"
#include "nlohmann/json.hpp"

using nlohmann::json;

bool Type::operator==(const Type &other) const {
  if (name != other.name)
    return false;
  if (param_type == nullptr || other.param_type == nullptr)
    return param_type == other.param_type;
  return *param_type == *other.param_type;
}

void to_json(json &j, const Type &t) {
  if (t.param_type)
    j = json{{t.name, *t.param_type}};
  else
    j = t.name;
}

void from_json(const json &j, Type &t) {
  if (j.is_string()) {
    t.name = j.get<std::string>();
    t.param_type = nullptr;
  } else if (j.is_object()) {
    auto it = j.begin();
    t.name = it.key();
    t.param_type = std::make_shared<Type>();
    from_json(it.value(), *t.param_type);
  } else
    throw std::runtime_error("Invalid type format");
}

void to_json(json &j, const Label &l) { j["label"] = l.name; }

void from_json(const json &j, Label &l) {
  l.name = j["label"].get<std::string>();
}

void to_json(json &j, const ValueOp &op) {
  static const std::unordered_map<ValueOpKind, std::string> op_map = {
      {ValueOpKind::Add, "add"},   {ValueOpKind::Mul, "mul"},
      {ValueOpKind::Sub, "sub"},   {ValueOpKind::Div, "div"},
      {ValueOpKind::Eq, "eq"},     {ValueOpKind::Lt, "lt"},
      {ValueOpKind::Gt, "gt"},     {ValueOpKind::Le, "le"},
      {ValueOpKind::Ge, "ge"},     {ValueOpKind::Not, "not"},
      {ValueOpKind::And, "and"},   {ValueOpKind::Or, "or"},
      {ValueOpKind::Call, "call"}, {ValueOpKind::Id, "id"}};

  j["op"] = op_map.at(op.kind);
  j["dest"] = op.dest;
  j["type"] = op.type;
  if (!op.args.empty())
    j["args"] = op.args;
  if (!op.funcs.empty())
    j["funcs"] = op.funcs;
  if (!op.labels.empty())
    j["labels"] = op.labels;
}

void from_json(const json &j, ValueOp &op) {
  static const std::unordered_map<std::string, ValueOpKind> op_map = {
      {"add", ValueOpKind::Add},   {"mul", ValueOpKind::Mul},
      {"sub", ValueOpKind::Sub},   {"div", ValueOpKind::Div},
      {"eq", ValueOpKind::Eq},     {"lt", ValueOpKind::Lt},
      {"gt", ValueOpKind::Gt},     {"le", ValueOpKind::Le},
      {"ge", ValueOpKind::Ge},     {"not", ValueOpKind::Not},
      {"and", ValueOpKind::And},   {"or", ValueOpKind::Or},
      {"call", ValueOpKind::Call}, {"id", ValueOpKind::Id}};

  op.kind = op_map.at(j["op"].get<std::string>());
  op.dest = j["dest"].get<std::string>();
  j["type"].get_to(op.type);
  if (j.contains("args"))
    j["args"].get_to(op.args);
  if (j.contains("funcs"))
    j["funcs"].get_to(op.funcs);
  if (j.contains("labels"))
    j["labels"].get_to(op.labels);
}

void to_json(json &j, const EffectOp &op) {
  static const std::unordered_map<EffectOpKind, std::string> op_map = {
      {EffectOpKind::Jmp, "jmp"},
      {EffectOpKind::Br, "br"},
      {EffectOpKind::Call, "call"},
      {EffectOpKind::Ret, "ret"},
      {EffectOpKind::Print, "print"}};

  j["op"] = op_map.at(op.kind);
  if (!op.args.empty())
    j["args"] = op.args;
  if (!op.funcs.empty())
    j["funcs"] = op.funcs;
  if (!op.labels.empty())
    j["labels"] = op.labels;
}

void from_json(const json &j, EffectOp &op) {
  static const std::unordered_map<std::string, EffectOpKind> op_map = {
      {"jmp", EffectOpKind::Jmp},
      {"br", EffectOpKind::Br},
      {"call", EffectOpKind::Call},
      {"ret", EffectOpKind::Ret},
      {"print", EffectOpKind::Print}};

  op.kind = op_map.at(j["op"].get<std::string>());
  if (j.contains("args"))
    j["args"].get_to(op.args);
  if (j.contains("funcs"))
    j["funcs"].get_to(op.funcs);
  if (j.contains("labels"))
    j["labels"].get_to(op.labels);
}

void to_json(nlohmann::json &j, const Constant &c) {
  j["op"] = "const";
  j["type"] = c.type().name;
  j["dest"] = c.dest;
  std::visit([&j](auto &&arg) { j["value"] = arg; }, c.value);
}

void from_json(const nlohmann::json &j, Constant &c) {
  j["dest"].get_to(c.dest);
  auto type_name = j["type"];
  if (type_name == "int")
    c.value = j["value"].get<int64_t>();
  else if (type_name == "bool")
    c.value = j["value"].get<bool>();
  else
    throw std::runtime_error(
        fmt::format("Invalid constant type: {}", std::string(type_name)));
}

std::optional<ValueOp> try_op_into_value_op(const Op &op) {
  return std::visit(
      match{
          [](const ValueOp &value_op) { return std::optional{value_op}; },
          [](auto) { return std::optional<ValueOp>{}; },
      },
      op);
}

std::optional<EffectOp> try_op_into_effect_op(const Op &op) {
  return std::visit(
      match{
          [](const EffectOp &effect_op) { return std::optional{effect_op}; },
          [](auto) { return std::optional<EffectOp>{}; },
      },
      op);
}

std::optional<Constant> try_op_into_constant(const Op &op) {
  return std::visit(
      match{
          [](const Constant &constant) { return std::optional{constant}; },
          [](auto) { return std::optional<Constant>{}; },
      },
      op);
}

void to_json(json &j, const Op &op) {
  std::visit([&j](auto &&arg) { to_json(j, arg); }, op);
}

void from_json(const json &j, Op &op) {
  if (j["op"] == "const")
    op = j.get<Constant>();
  else if (j.contains("dest"))
    op = j.get<ValueOp>();
  else
    op = j.get<EffectOp>();
}

void to_json(json &j, const Argument &arg) {
  j = json{{"name", arg.name}, {"type", arg.type}};
}

void from_json(const json &j, Argument &arg) {
  j["name"].get_to(arg.name);
  j["type"].get_to(arg.type);
}

void to_json(json &j, const Instruction &instr) {
  std::visit([&j](auto &&arg) { to_json(j, arg); }, instr);
}

void from_json(const json &j, Instruction &instr) {
  if (j.contains("label"))
    instr = j.get<Label>();
  else
    instr = j.get<Op>();
}

std::optional<Op> try_instr_into_op(const Instruction &instr) {
  return std::visit(match{
                        [](const Op &op) { return std::optional{op}; },
                        [](auto) { return std::optional<Op>{}; },
                    },
                    instr);
}

std::optional<Label> try_instr_into_label(const Instruction &instr) {
  return std::visit(match{
                        [](const Label &label) { return std::optional{label}; },
                        [](auto) { return std::optional<Label>{}; },
                    },
                    instr);
}

void to_json(json &j, const Function &func) {
  j["name"] = func.name;
  if (!func.args.empty())
    j["args"] = func.args;
  if (func.return_type)
    j["type"] = *func.return_type;
  j["instrs"] = func.instrs;
}

void from_json(const json &j, Function &func) {
  j["name"].get_to(func.name);
  if (j.contains("args"))
    j["args"].get_to(func.args);
  if (j.contains("type"))
    func.return_type = j["type"].get<Type>();
  else
    func.return_type = std::nullopt;
  j["instrs"].get_to(func.instrs);
}

void to_json(json &j, const Program &prog) { j["functions"] = prog.functions; }

void from_json(const json &j, Program &prog) {
  j["functions"].get_to(prog.functions);
}
