// matching engine tests
//
// run just these: ./tests/exchange_tests "[matching]"

#include <catch2/catch_test_macros.hpp>

#include "exchange/matching_engine.hpp"
#include "exchange/order.hpp"
#include "exchange/time/timestamp.hpp"
#include "exchange/trade.hpp"
#include "exchange/types.hpp"
#include "test_log.hpp"

using namespace exchange;

namespace {
Order limit(std::uint64_t id, Side side, std::int64_t px, std::int64_t qty) {
    return Order::from_new_order(OrderId{id}, side, OrderType::Limit,
                                 Price{px}, Qty{qty}, now());
}
Order market(std::uint64_t id, Side side, std::int64_t qty) {
    return Order::from_new_order(OrderId{id}, side, OrderType::Market,
                                 Price{0}, Qty{qty}, now());
}
}  // namespace

TEST_CASE("non-crossing limit order just rests, no trade", "[matching]") {
    LOG_TEST("non-crossing limit rests");
    MatchingEngine eng;

    // bid at 100, then a sell at 105 — no overlap
    auto t1 = eng.submit(limit(1, Side::Buy, 100, 5));
    auto t2 = eng.submit(limit(2, Side::Sell, 105, 5));

    LOG_KV("trades from bid", t1.size());
    LOG_KV("trades from ask", t2.size());
    REQUIRE(t1.empty());
    REQUIRE(t2.empty());
    REQUIRE(eng.book().best_bid()->value == 100);
    REQUIRE(eng.book().best_ask()->value == 105);
}

TEST_CASE("crossing limit order fully fills and removes maker", "[matching]") {
    LOG_TEST("crossing limit fully fills");
    MatchingEngine eng;

    eng.submit(limit(1, Side::Sell, 100, 5));   // resting ask @ 100
    auto trades = eng.submit(limit(2, Side::Buy, 100, 5));  // buy @ 100 hits it

    REQUIRE(trades.size() == 1);
    LOG_KV("trade price", trades[0].price.value);
    LOG_KV("trade qty", trades[0].qty.value);
    LOG_KV("taker", trades[0].taker_id.value);
    LOG_KV("maker", trades[0].maker_id.value);

    REQUIRE(trades[0].price.value == 100);   // maker's price
    REQUIRE(trades[0].qty.value == 5);
    REQUIRE(trades[0].taker_id.value == 2);
    REQUIRE(trades[0].maker_id.value == 1);

    // book is now empty — both orders gone
    REQUIRE(eng.book().order_count() == 0);
}

TEST_CASE("trade executes at maker price, not taker price", "[matching]") {
    LOG_TEST("trade at maker price");
    MatchingEngine eng;

    eng.submit(limit(1, Side::Sell, 100, 5));        // ask @ 100
    auto trades = eng.submit(limit(2, Side::Buy, 110, 5));  // willing to pay up to 110

    // should fill at 100 (maker), buyer gets price improvement
    REQUIRE(trades.size() == 1);
    LOG_KV("executed price", trades[0].price.value);
    REQUIRE(trades[0].price.value == 100);
}

TEST_CASE("taker larger than maker leaves remainder resting", "[matching]") {
    LOG_TEST("taker remainder rests");
    MatchingEngine eng;

    eng.submit(limit(1, Side::Sell, 100, 3));        // only 3 available
    auto trades = eng.submit(limit(2, Side::Buy, 100, 8));  // wants 8

    REQUIRE(trades.size() == 1);
    REQUIRE(trades[0].qty.value == 3);               // filled what was there
    LOG_KV("remaining bid qty", eng.book().qty_at_price(Side::Buy, Price{100}).value);
    // leftover 5 rests as a bid @ 100
    REQUIRE(eng.book().best_bid()->value == 100);
    REQUIRE(eng.book().qty_at_price(Side::Buy, Price{100}).value == 5);
}

TEST_CASE("market order sweeps multiple ask levels", "[matching]") {
    LOG_TEST("market order sweeps levels");
    MatchingEngine eng;

    eng.submit(limit(1, Side::Sell, 100, 2));
    eng.submit(limit(2, Side::Sell, 101, 2));
    eng.submit(limit(3, Side::Sell, 102, 2));

    // market buy for 5 should eat 100(2), 101(2), 102(1)
    auto trades = eng.submit(market(4, Side::Buy, 5));

    LOG_KV("number of trades", trades.size());
    REQUIRE(trades.size() == 3);
    REQUIRE(trades[0].price.value == 100);
    REQUIRE(trades[1].price.value == 101);
    REQUIRE(trades[2].price.value == 102);
    REQUIRE(trades[2].qty.value == 1);

    // 1 left at 102, nothing rests from a market order
    REQUIRE(eng.book().qty_at_price(Side::Sell, Price{102}).value == 1);
    REQUIRE(eng.book().best_ask()->value == 102);
}

TEST_CASE("price-time priority hits oldest order first", "[matching]") {
    LOG_TEST("price-time priority oldest first");
    MatchingEngine eng;

    // two asks at the SAME price — order 1 is older
    eng.submit(limit(1, Side::Sell, 100, 3));
    eng.submit(limit(2, Side::Sell, 100, 3));

    // buy 3 — should hit order 1 (oldest) only
    auto trades = eng.submit(limit(3, Side::Buy, 100, 3));

    REQUIRE(trades.size() == 1);
    LOG_KV("maker filled", trades[0].maker_id.value);
    REQUIRE(trades[0].maker_id.value == 1);   // oldest, not 2

    // order 2 still rests
    REQUIRE(eng.book().qty_at_price(Side::Sell, Price{100}).value == 3);
    REQUIRE(eng.book().front_id(Side::Sell).value == 2);
}

TEST_CASE("market order with empty book produces no trades", "[matching]") {
    LOG_TEST("market order on empty book");
    MatchingEngine eng;

    auto trades = eng.submit(market(1, Side::Buy, 10));
    LOG_KV("trades", trades.size());
    REQUIRE(trades.empty());
    REQUIRE(eng.book().order_count() == 0);  // market order does not rest
}
