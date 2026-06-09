cmake_minimum_required(VERSION 3.20)
project(latency_exchange VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CompilerWarnings)

add_executable(latency_exchange src/main.cpp)
set_project_warnings(latency_exchange)

enable_testing()
add_subdirectory(tests)
