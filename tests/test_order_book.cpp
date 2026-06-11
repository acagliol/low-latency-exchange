// order book tests — TDD style
//
// this test is INTENTIONALLY FAILING until day 3 implements OrderBook::add
// ctest will show: 13 pass, 1 fail — that's expected
//
// tagged [pending] so you can filter: ./exchange_tests "[orderbook]"

#include <catch2/catch_test_macros.hpp>

#include "exchange/order.hpp"
#include "exchange/order_book.hpp"
#include "exchange/time/timestamp.hpp"
#include "exchange/types.hpp"
#include "test_log.hpp"

using namespace exchange;

TEST_CASE("order book tracks best bid and offer", "[orderbook][pending]") {
    LOG_TEST("order book tracks best bid and offer (PENDING day 3)");
    OrderBook book;

    // one bid at 100, one ask at 105 — classic simple book
    const auto bid = Order::from_new_order(
        OrderId{1}, Side::Buy, OrderType::Limit, Price{100}, Qty{50}, now());
    const auto ask = Order::from_new_order(
        OrderId{2}, Side::Sell, OrderType::Limit, Price{105}, Qty{30}, now());

    LOG("adding bid @ 100 and ask @ 105");
    LOG_KV("book.add(bid) returned", book.add(bid));
    LOG_KV("book.add(ask) returned", book.add(ask));
    LOG("^ both false right now because add() is a stub — day 3 fixes this");

    // day 3: these should return true once add() is implemented
    REQUIRE(book.add(bid));
    REQUIRE(book.add(ask));

    // BBO = best bid/offer — the prices a market order would hit
    REQUIRE(book.best_bid().has_value());
    REQUIRE(book.best_ask().has_value());
    REQUIRE(book.best_bid()->value == 100);
    REQUIRE(book.best_ask()->value == 105);
    REQUIRE(book.bid_levels() == 1);
    REQUIRE(book.ask_levels() == 1);
}
