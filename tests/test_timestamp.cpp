#include <catch2/catch_test_macros.hpp>

#include "exchange/time/timestamp.hpp"

using namespace exchange;

TEST_CASE("timestamp delta is non-negative forward in time", "[timestamp]") {
    const auto t0 = now();
    const auto t1 = now();

    REQUIRE(t1.elapsed_ns(t0) >= 0);
}

TEST_CASE("timestamp comparison works", "[timestamp]") {
    const Timestamp a{100};
    const Timestamp b{200};

    REQUIRE(a < b);
    REQUIRE(a.elapsed_ns(b) == -100);
}
