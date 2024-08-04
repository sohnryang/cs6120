#pragma once

#include "bril_types.hh"

#include <string>

#include <simdjson.h>

class BrilParser {
private:
  simdjson::padded_string padded_json;

public:
  BrilParser(const std::string &json_content);

  simdjson::simdjson_result<Program> parse();
};
