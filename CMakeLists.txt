cmake_minimum_required(VERSION 3.20)
project(latency_exchange VERSION 0.1.0 LANGUAGES CXX)

# c++20 for <=> spaceship, concepts, span — all useful later
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# helps clangd / ide jump-to-def from build dir
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CompilerWarnings)

# header-only lib — no .cpp, just include path propagation
# anything that needs exchange headers links this (tests, feed, engine, etc.)
add_library(exchange_core INTERFACE)
target_include_directories(exchange_core INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# compiled exchange logic (book, matcher, feed — grows over time)
add_library(exchange_lib STATIC
  src/order_book.cpp
  src/matching_engine.cpp
)
target_link_libraries(exchange_lib PUBLIC exchange_core)
set_project_warnings(exchange_lib)

# main binary — will grow into process entry point later
add_executable(latency_exchange src/main.cpp)
target_link_libraries(latency_exchange PRIVATE exchange_lib)
set_project_warnings(latency_exchange)

enable_testing()
add_subdirectory(tests)
