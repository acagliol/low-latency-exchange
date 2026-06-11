#pragma once

// tiny logging helper for tests
// catch2's INFO/CAPTURE only print on failure — these print ALWAYS
// so you can physically watch what a test does when you run it
//
// run a single test with full detail:
//   ./tests/exchange_tests "name of test"        # shows our [log] lines
//   ./tests/exchange_tests "name of test" -s     # also shows passing REQUIRE checks

#include <iostream>

// section banner — call once at the top of a TEST_CASE
#define LOG_TEST(name) \
    std::cout << "\n=== " << name << " ===\n"

// plain message line
#define LOG(msg) \
    std::cout << "  [log] " << msg << "\n"

// key = value line for inspecting fields
#define LOG_KV(key, val) \
    std::cout << "  [log] " << key << " = " << (val) << "\n"
