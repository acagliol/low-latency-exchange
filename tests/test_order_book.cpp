#include <catch2/catch_test_macros.hpp>

#include "exchange/order.hpp"
#include "exchange/order_book.hpp"
#include "exchange/time/timestamp.hpp"
#include "exchange/types.hpp"

using namespace exchange;

TEST_CASE("order book tracks best bid and offer", "[orderbook][pending]") {
    OrderBook book;

    const auto bid = Order::from_new_order(
        OrderId{1}, Side::Buy, OrderType::Limit, Price{100}, Qty{50}, now());
    const auto ask = Order::from_new_order(
        OrderId{2}, Side::Sell, OrderType::Limit, Price{105}, Qty{30}, now());

    REQUIRE(book.add(bid));
    REQUIRE(book.add(ask));

    REQUIRE(book.best_bid().has_value());
    REQUIRE(book.best_ask().has_value());
    REQUIRE(book.best_bid()->value == 100);
    REQUIRE(book.best_ask()->value == 105);
    REQUIRE(book.bid_levels() == 1);
    REQUIRE(book.ask_levels() == 1);
}
