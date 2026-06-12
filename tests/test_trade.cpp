// tests for the Trade record

#include <catch2/catch_test_macros.hpp>

#include "exchange/trade.hpp"
#include "exchange/time/timestamp.hpp"
#include "exchange/types.hpp"
#include "test_log.hpp"

using namespace exchange;

TEST_CASE("trade holds taker, maker, price and qty", "[trade]") {
    LOG_TEST("trade holds taker, maker, price and qty");

    const Trade t{OrderId{10}, OrderId{7}, Price{100}, Qty{25}, Timestamp{500}};

    LOG_KV("taker_id", t.taker_id.value);
    LOG_KV("maker_id", t.maker_id.value);
    LOG_KV("price", t.price.value);
    LOG_KV("qty", t.qty.value);

    REQUIRE(t.taker_id.value == 10);
    REQUIRE(t.maker_id.value == 7);
    REQUIRE(t.price.value == 100);
    REQUIRE(t.qty.value == 25);
    REQUIRE(t.ts.ns == 500);
}
