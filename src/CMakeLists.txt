include(FetchContent)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 11.0.2)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(
  json
  URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
FetchContent_MakeAvailable(json)

FetchContent_Declare(
  argparse
  GIT_REPOSITORY https://github.com/p-ranav/argparse.git
  GIT_TAG v3.1)
FetchContent_MakeAvailable(argparse)

add_library(bril_types bril_types.cc)
target_include_directories(bril_types PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(bril_types PUBLIC nlohmann_json::nlohmann_json PRIVATE fmt::fmt)

add_library(analysis cfg.cc local_dce.cc dump_cfg.cc dom.cc natural_loop.cc graph.cc reaching_def.cc)
target_include_directories(analysis PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(analysis PUBLIC bril_types PRIVATE fmt::fmt)

add_executable(opt opt.cc)
target_include_directories(opt PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(opt PRIVATE analysis PRIVATE fmt::fmt PRIVATE argparse)
