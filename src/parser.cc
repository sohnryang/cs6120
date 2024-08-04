#include "parser.hh"
#include "bril_types.hh"

#include <string>

#include <simdjson.h>

template <>
simdjson_inline simdjson::simdjson_result<Type>
simdjson::ondemand::value::get<Type>() noexcept {
  if (is_string()) {
    auto str = get_string();
    if (str.error())
      return str.error();
    return Type{.name = std::string(str.value()), .param_type = nullptr};
  }

  auto obj = get_object();
  if (obj.error())
    return obj.error();

  for (auto field : obj.value()) {
    auto key = field.unescaped_key();
    if (key.error())
      return key.error();
    auto value = field.value().get<Type>();
    if (value.error())
      return value.error();
    return Type{.name = std::string(key.value()),
                .param_type = std::make_shared<Type>(value.value())};
  }
  return simdjson::INCORRECT_TYPE;
}

template <>
simdjson_inline simdjson::simdjson_result<ValueOp>
simdjson::ondemand::value::get<ValueOp>() noexcept {
  auto obj = get_object();
  if (obj.error())
    return obj.error();

  ValueOp op;
  auto op_str = obj.value()["op"].get_string();
  if (op_str.error())
    return op_str.error();

  static const std::unordered_map<std::string_view, ValueOpKind> op_map = {
      {"add", ValueOpKind::Add},   {"mul", ValueOpKind::Mul},
      {"sub", ValueOpKind::Sub},   {"div", ValueOpKind::Div},
      {"eq", ValueOpKind::Eq},     {"lt", ValueOpKind::Lt},
      {"gt", ValueOpKind::Gt},     {"le", ValueOpKind::Le},
      {"ge", ValueOpKind::Ge},     {"not", ValueOpKind::Not},
      {"and", ValueOpKind::And},   {"or", ValueOpKind::Or},
      {"call", ValueOpKind::Call},
  };
  op.kind = op_map.at(op_str.value());

  auto dest = obj.value()["dest"].get_string();
  if (dest.error())
    return dest.error();
  op.dest = std::string(dest.value());

  auto type = obj.value()["type"].get<Type>();
  if (type.error())
    return type.error();
  op.type = type.value();

  auto args = obj.value()["args"].get_array();
  if (!args.error()) {
    for (auto arg : args.value()) {
      auto arg_str = arg.get_string();
      if (arg_str.error())
        return arg_str.error();
      op.args.push_back(std::string(arg_str.value()));
    }
  }

  auto funcs = obj.value()["funcs"].get_array();
  if (!funcs.error()) {
    for (auto func : funcs.value()) {
      auto func_str = func.get_string();
      if (func_str.error())
        return func_str.error();
      op.funcs.push_back(std::string(func_str.value()));
    }
  }

  auto labels = obj.value()["labels"].get_array();
  if (!labels.error()) {
    for (auto label : labels.value()) {
      auto label_str = label.get_string();
      if (label_str.error())
        return label_str.error();
      op.labels.push_back(std::string(label_str.value()));
    }
  }

  return op;
}

template <>
simdjson_inline simdjson::simdjson_result<EffectOp>
simdjson::ondemand::value::get<EffectOp>() noexcept {
  auto obj = get_object();
  if (obj.error())
    return obj.error();

  EffectOp op;
  auto op_str = obj.value()["op"].get_string();
  if (op_str.error())
    return op_str.error();

  static const std::unordered_map<std::string_view, EffectOpKind> op_map = {
      {"jmp", EffectOpKind::Jmp},
      {"br", EffectOpKind::Br},
      {"call", EffectOpKind::Call},
      {"ret", EffectOpKind::Ret},
  };
  op.kind = op_map.at(op_str.value());

  auto args = obj.value()["args"].get_array();
  if (!args.error()) {
    for (auto arg : args.value()) {
      auto arg_str = arg.get_string();
      if (arg_str.error())
        return arg_str.error();
      op.args.push_back(std::string(arg_str.value()));
    }
  }

  auto funcs = obj.value()["funcs"].get_array();
  if (!funcs.error()) {
    for (auto func : funcs.value()) {
      auto func_str = func.get_string();
      if (func_str.error())
        return func_str.error();
      op.funcs.push_back(std::string(func_str.value()));
    }
  }

  auto labels = obj.value()["labels"].get_array();
  if (!labels.error()) {
    for (auto label : labels.value()) {
      auto label_str = label.get_string();
      if (label_str.error())
        return label_str.error();
      op.labels.push_back(std::string(label_str.value()));
    }
  }

  return op;
}
template <>
simdjson_inline simdjson::simdjson_result<Constant>
simdjson::ondemand::value::get<Constant>() noexcept {
  auto obj = get_object();
  if (obj.error())
    return obj.error();

  auto type = obj.value()["type"].get_string();
  if (type.error())
    return type.error();

  if (std::string(type.value()) == "int") {
    auto val = obj.value()["value"].get_int64();
    if (val.error())
      return val.error();
    return Constant{.value = val.value()};
  } else if (std::string(type.value()) == "bool") {
    auto val = obj.value()["value"].get_bool();
    if (val.error())
      return val.error();
    return Constant{.value = val.value()};
  }
  return simdjson::INCORRECT_TYPE;
}

template <>
simdjson_inline simdjson::simdjson_result<Instruction>
simdjson::ondemand::value::get<Instruction>() noexcept {
  auto obj = get_object();
  if (obj.error())
    return obj.error();

  if (obj["label"].error() != simdjson::error_code::NO_SUCH_FIELD) {
    auto label = obj["label"].get_string();
    if (label.error())
      return label.error();
    return {Label{.name = std::string(label.value())}};
  }

  auto op = obj["op"].get_string();
  if (op.error())
    return op.error();

  if (std::string_view(op.value()) == "const") {
    auto constant = get<Constant>();
    if (constant.error())
      return constant.error();
    return {Op{constant.value()}};
  } else if (obj["dest"].error() != simdjson::error_code::NO_SUCH_FIELD) {
    auto value_op = get<ValueOp>();
    if (value_op.error())
      return value_op.error();
    return {Op{value_op.value()}};
  } else {
    auto effect_op = get<EffectOp>();
    if (effect_op.error())
      return effect_op.error();
    return {Op{effect_op.value()}};
  }
}

template <>
simdjson_inline simdjson::simdjson_result<Function>
simdjson::ondemand::value::get<Function>() noexcept {
  auto obj = get_object();
  if (obj.error())
    return obj.error();

  auto name = obj["name"].get_string();
  if (name.error())
    return name.error();

  Function function;
  function.name = std::string(name.value());

  auto args = obj["args"].get_array();
  if (!args.error()) {
    for (auto arg : args.value()) {
      auto arg_obj = arg.get_object();
      if (arg_obj.error())
        return arg_obj.error();

      auto arg_name = arg_obj.value()["name"].get_string();
      if (arg_name.error())
        return arg_name.error();

      auto arg_type = arg_obj.value()["type"].get<Type>();
      if (arg_type.error())
        return arg_type.error();

      function.args.push_back(Argument{.name = std::string(arg_name.value()),
                                       .type = arg_type.value()});
    }
  }

  auto return_type = obj["type"].get<Type>();
  if (!return_type.error()) {
    function.return_type = return_type.value();
  }

  auto instrs = obj["instrs"].get_array();
  if (instrs.error())
    return instrs.error();
  for (auto instr : instrs.value()) {
    auto instr_obj = instr.get<Instruction>();
    if (instr_obj.error())
      return instr_obj.error();
    function.instrs.push_back(instr_obj.value());
  }

  return function;
}

template <>
simdjson_inline simdjson::simdjson_result<Program>
simdjson::ondemand::value::get<Program>() noexcept {
  ondemand::object obj;
  auto error = get(obj);
  if (error) {
    return error;
  }

  Program program;
  auto functions = obj["functions"].get_array();
  if (functions.error()) {
    return functions.error();
  }
  for (auto function : functions) {
    auto func = function.get<Function>();
    if (func.error()) {
      return func.error();
    }
    program.functions.push_back(func.value());
  }

  return program;
}

BrilParser::BrilParser(const std::string &json_content)
    : padded_json(json_content) {}

simdjson::simdjson_result<Program> BrilParser::parse() {
  simdjson::ondemand::parser parser;
  auto doc = parser.iterate(padded_json);
  return doc.get<Program>();
}
