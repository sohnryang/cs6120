include(FetchContent)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 11.0.2)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(
  simdjson
  GIT_REPOSITORY https://github.com/simdjson/simdjson.git
  GIT_SHALLOW TRUE
  GIT_TAG v3.10.0)
FetchContent_MakeAvailable(simdjson)

add_library(parser parser.cc)
target_include_directories(parser PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(parser PRIVATE simdjson)
