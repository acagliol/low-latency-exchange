// tests for in-memory Order struct
// checks that from_new_order sets fields correctly

#include <catch2/catch_test_macros.hpp>

#include "exchange/order.hpp"
#include "exchange/time/timestamp.hpp"
#include "exchange/types.hpp"
#include "test_log.hpp"

using namespace exchange;

TEST_CASE("order stores remaining qty equal to original", "[order]") {
    LOG_TEST("order stores remaining qty equal to original");

    // fixed timestamp so test doesn't depend on clock
    const auto ts = Timestamp{1000};

    const auto o = Order::from_new_order(
        OrderId{42}, Side::Buy, OrderType::Limit, Price{500}, Qty{100}, ts);

    LOG_KV("id", o.id.value);
    LOG_KV("price", o.price.value);
    LOG_KV("qty", o.qty.value);
    LOG_KV("remaining", o.remaining.value);
    LOG_KV("created_at.ns", o.created_at.ns);

    REQUIRE(o.id.value == 42);
    REQUIRE(o.qty.value == 100);
    REQUIRE(o.remaining.value == 100);  // nothing filled yet
    REQUIRE(o.created_at.ns == 1000);
}
