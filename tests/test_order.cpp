#include <catch2/catch_test_macros.hpp>

#include "exchange/order.hpp"
#include "exchange/time/timestamp.hpp"
#include "exchange/types.hpp"

using namespace exchange;

TEST_CASE("order stores remaining qty equal to original", "[order]") {
    const auto ts = Timestamp{1000};
    const auto o  = Order::from_new_order(
        OrderId{42}, Side::Buy, OrderType::Limit, Price{500}, Qty{100}, ts);

    REQUIRE(o.id.value == 42);
    REQUIRE(o.qty.value == 100);
    REQUIRE(o.remaining.value == 100);
    REQUIRE(o.created_at.ns == 1000);
}
