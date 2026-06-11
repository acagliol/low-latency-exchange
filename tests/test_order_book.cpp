// order book tests
//
// run just these: ./tests/exchange_tests "[orderbook]"

#include <catch2/catch_test_macros.hpp>

#include "exchange/order.hpp"
#include "exchange/order_book.hpp"
#include "exchange/time/timestamp.hpp"
#include "exchange/types.hpp"
#include "test_log.hpp"

using namespace exchange;

TEST_CASE("order book tracks best bid and offer", "[orderbook]") {
    LOG_TEST("order book tracks best bid and offer");
    OrderBook book;

    // one bid at 100, one ask at 105 — classic simple book
    const auto bid = Order::from_new_order(
        OrderId{1}, Side::Buy, OrderType::Limit, Price{100}, Qty{50}, now());
    const auto ask = Order::from_new_order(
        OrderId{2}, Side::Sell, OrderType::Limit, Price{105}, Qty{30}, now());

    LOG("adding bid @ 100 and ask @ 105");
    const bool added_bid = book.add(bid);
    const bool added_ask = book.add(ask);
    LOG_KV("add(bid) returned", added_bid);
    LOG_KV("add(ask) returned", added_ask);
    REQUIRE(added_bid);
    REQUIRE(added_ask);

    // BBO = best bid/offer — the prices a market order would hit
    REQUIRE(book.best_bid().has_value());
    REQUIRE(book.best_ask().has_value());
    LOG_KV("best_bid", book.best_bid()->value);
    LOG_KV("best_ask", book.best_ask()->value);
    REQUIRE(book.best_bid()->value == 100);
    REQUIRE(book.best_ask()->value == 105);
    REQUIRE(book.bid_levels() == 1);
    REQUIRE(book.ask_levels() == 1);
}

TEST_CASE("order book rejects market orders and duplicates", "[orderbook]") {
    LOG_TEST("order book rejects market orders and duplicates");
    OrderBook book;

    // market orders don't rest on the book
    const auto mkt = Order::from_new_order(
        OrderId{1}, Side::Buy, OrderType::Market, Price{0}, Qty{10}, now());
    LOG_KV("add(market) returned", book.add(mkt));
    REQUIRE_FALSE(book.add(mkt));

    // first limit order accepted, same id again rejected
    const auto lim = Order::from_new_order(
        OrderId{2}, Side::Buy, OrderType::Limit, Price{100}, Qty{10}, now());
    REQUIRE(book.add(lim));
    LOG_KV("add(duplicate id) returned", book.add(lim));
    REQUIRE_FALSE(book.add(lim));
}

TEST_CASE("best bid picks highest, best ask picks lowest", "[orderbook]") {
    LOG_TEST("best bid highest, best ask lowest");
    OrderBook book;

    // three bids at different prices — best should be the highest (102)
    book.add(Order::from_new_order(OrderId{1}, Side::Buy, OrderType::Limit, Price{100}, Qty{1}, now()));
    book.add(Order::from_new_order(OrderId{2}, Side::Buy, OrderType::Limit, Price{102}, Qty{1}, now()));
    book.add(Order::from_new_order(OrderId{3}, Side::Buy, OrderType::Limit, Price{101}, Qty{1}, now()));

    // three asks — best should be the lowest (105)
    book.add(Order::from_new_order(OrderId{4}, Side::Sell, OrderType::Limit, Price{107}, Qty{1}, now()));
    book.add(Order::from_new_order(OrderId{5}, Side::Sell, OrderType::Limit, Price{105}, Qty{1}, now()));
    book.add(Order::from_new_order(OrderId{6}, Side::Sell, OrderType::Limit, Price{106}, Qty{1}, now()));

    LOG_KV("best_bid", book.best_bid()->value);
    LOG_KV("best_ask", book.best_ask()->value);
    REQUIRE(book.best_bid()->value == 102);
    REQUIRE(book.best_ask()->value == 105);
    REQUIRE(book.bid_levels() == 3);
    REQUIRE(book.ask_levels() == 3);
}
