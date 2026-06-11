// tests for Timestamp / now()
// sanity checks — real latency benchmarks come much later in bench/

#include <catch2/catch_test_macros.hpp>

#include "exchange/time/timestamp.hpp"
#include "test_log.hpp"

using namespace exchange;

TEST_CASE("timestamp delta is non-negative forward in time", "[timestamp]") {
    LOG_TEST("timestamp delta is non-negative");

    // steady_clock should never go backward between two consecutive now() calls
    const auto t0 = now();
    const auto t1 = now();

    const auto delta = t1.elapsed_ns(t0);
    LOG_KV("t0.ns", t0.ns);
    LOG_KV("t1.ns", t1.ns);
    LOG_KV("delta ns", delta);

    REQUIRE(delta >= 0);
}

TEST_CASE("timestamp comparison works", "[timestamp]") {
    LOG_TEST("timestamp comparison works");

    // manual values — not calling now() so test is deterministic
    const Timestamp a{100};
    const Timestamp b{200};

    LOG_KV("a.ns", a.ns);
    LOG_KV("b.ns", b.ns);
    LOG_KV("a.elapsed_ns(b)", a.elapsed_ns(b));

    REQUIRE(a < b);
    REQUIRE(a.elapsed_ns(b) == -100);  // a is 100ns before b
}
