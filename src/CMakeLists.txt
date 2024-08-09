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

add_library(bril_types bril_types.cc)
target_include_directories(bril_types PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(bril_types PUBLIC nlohmann_json::nlohmann_json PRIVATE fmt::fmt)
