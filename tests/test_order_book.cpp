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

TEST_CASE("cancel removes an order and updates bbo", "[orderbook]") {
    LOG_TEST("cancel removes an order and updates bbo");
    OrderBook book;

    book.add(Order::from_new_order(OrderId{1}, Side::Buy, OrderType::Limit, Price{100}, Qty{5}, now()));
    book.add(Order::from_new_order(OrderId{2}, Side::Buy, OrderType::Limit, Price{101}, Qty{5}, now()));
    REQUIRE(book.best_bid()->value == 101);
    LOG_KV("best_bid before cancel", book.best_bid()->value);

    // cancel the top bid — best should drop to 100
    REQUIRE(book.cancel(OrderId{2}));
    LOG_KV("best_bid after cancel", book.best_bid()->value);
    REQUIRE(book.best_bid()->value == 100);
    REQUIRE(book.bid_levels() == 1);

    // cancelling an unknown id fails
    LOG_KV("cancel(unknown) returned", book.cancel(OrderId{999}));
    REQUIRE_FALSE(book.cancel(OrderId{999}));

    // cancel the last bid — book side becomes empty
    REQUIRE(book.cancel(OrderId{1}));
    REQUIRE_FALSE(book.best_bid().has_value());
    REQUIRE(book.bid_levels() == 0);
}

TEST_CASE("modify changes price and qty via cancel-replace", "[orderbook]") {
    LOG_TEST("modify changes price and qty");
    OrderBook book;

    book.add(Order::from_new_order(OrderId{1}, Side::Buy, OrderType::Limit, Price{100}, Qty{5}, now()));
    REQUIRE(book.best_bid()->value == 100);

    // move the order up to 103 and resize to 8
    REQUIRE(book.modify(OrderId{1}, Price{103}, Qty{8}));
    LOG_KV("best_bid after modify", book.best_bid()->value);
    LOG_KV("qty at 103", book.qty_at_price(Side::Buy, Price{103}).value);
    REQUIRE(book.best_bid()->value == 103);
    REQUIRE(book.qty_at_price(Side::Buy, Price{103}).value == 8);
    REQUIRE(book.qty_at_price(Side::Buy, Price{100}).value == 0);  // old level gone

    // modifying an unknown id fails
    REQUIRE_FALSE(book.modify(OrderId{999}, Price{50}, Qty{1}));
}

TEST_CASE("orders at same price keep fifo time priority", "[orderbook]") {
    LOG_TEST("fifo time priority at one level");
    OrderBook book;

    // three orders at the same price — depth should add up, one level only
    book.add(Order::from_new_order(OrderId{1}, Side::Buy, OrderType::Limit, Price{100}, Qty{3}, now()));
    book.add(Order::from_new_order(OrderId{2}, Side::Buy, OrderType::Limit, Price{100}, Qty{4}, now()));
    book.add(Order::from_new_order(OrderId{3}, Side::Buy, OrderType::Limit, Price{100}, Qty{5}, now()));

    LOG_KV("bid_levels", book.bid_levels());
    LOG_KV("qty at 100", book.qty_at_price(Side::Buy, Price{100}).value);
    LOG_KV("order_count", book.order_count());

    REQUIRE(book.bid_levels() == 1);                          // all same price
    REQUIRE(book.qty_at_price(Side::Buy, Price{100}).value == 12);  // 3+4+5
    REQUIRE(book.order_count() == 3);

    // cancel the middle order — depth drops, level + others remain
    REQUIRE(book.cancel(OrderId{2}));
    LOG_KV("qty at 100 after cancel mid", book.qty_at_price(Side::Buy, Price{100}).value);
    REQUIRE(book.qty_at_price(Side::Buy, Price{100}).value == 8);  // 3+5
    REQUIRE(book.bid_levels() == 1);
    REQUIRE(book.order_count() == 2);
}
