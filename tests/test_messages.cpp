#include <cstring>

#include <catch2/catch_test_macros.hpp>

#include "exchange/protocol/messages.hpp"
#include "exchange/types.hpp"
#include "test_log.hpp"

using namespace exchange;
using namespace exchange::protocol;

TEST_CASE("message header is 4 bytes on wire", "[protocol]") {
    LOG_TEST("message header is 4 bytes");
    LOG_KV("sizeof(MessageHeader)", sizeof(MessageHeader));
    static_assert(sizeof(MessageHeader) == 4);
    REQUIRE(sizeof(MessageHeader) == 4);
}

TEST_CASE("wire struct sizes match v0 spec", "[protocol]") {
    LOG_TEST("wire struct sizes match v0 spec");
    LOG_KV("sizeof(NewOrderMsg)", sizeof(NewOrderMsg));
    LOG_KV("sizeof(CancelMsg)", sizeof(CancelMsg));
    LOG_KV("sizeof(ModifyMsg)", sizeof(ModifyMsg));
    REQUIRE(sizeof(NewOrderMsg) == 31);
    REQUIRE(sizeof(CancelMsg) == 12);
    REQUIRE(sizeof(ModifyMsg) == 28);
}

TEST_CASE("make_new_order fills header and payload", "[protocol]") {
    LOG_TEST("make_new_order fills header and payload");
    auto msg = make_new_order(OrderId{7}, Side::Sell, OrderType::Limit,
                              Price{999}, Qty{50});

    LOG_KV("hdr.version", static_cast<int>(msg.hdr.version));
    LOG_KV("hdr.length", msg.hdr.length);
    LOG_KV("order_id", msg.order_id.value);
    LOG_KV("price", msg.price.value);
    LOG_KV("qty", msg.qty.value);

    REQUIRE(msg.hdr.version == 1);
    REQUIRE(msg.hdr.type == MsgType::NewOrder);
    REQUIRE(msg.hdr.length == sizeof(NewOrderMsg));
    REQUIRE(msg.order_id.value == 7);
    REQUIRE(msg.side == Side::Sell);
    REQUIRE(msg.order_type == OrderType::Limit);
    REQUIRE(msg.price.value == 999);
    REQUIRE(msg.qty.value == 50);
}

TEST_CASE("make_cancel and make_modify set correct lengths", "[protocol]") {
    LOG_TEST("make_cancel and make_modify set lengths");

    auto cancel = make_cancel(OrderId{3});
    LOG_KV("cancel.hdr.length", cancel.hdr.length);
    REQUIRE(cancel.hdr.type == MsgType::Cancel);
    REQUIRE(cancel.hdr.length == sizeof(CancelMsg));
    REQUIRE(cancel.order_id.value == 3);

    auto modify = make_modify(OrderId{5}, Price{100}, Qty{200});
    LOG_KV("modify.hdr.length", modify.hdr.length);
    LOG_KV("modify.new_price", modify.new_price.value);
    LOG_KV("modify.new_qty", modify.new_qty.value);
    REQUIRE(modify.hdr.type == MsgType::Modify);
    REQUIRE(modify.hdr.length == sizeof(ModifyMsg));
    REQUIRE(modify.new_price.value == 100);
    REQUIRE(modify.new_qty.value == 200);
}

TEST_CASE("new order serializes version as first byte", "[protocol]") {
    LOG_TEST("new order serializes version as first byte");
    auto msg = make_new_order(OrderId{1}, Side::Buy, OrderType::Market,
                              Price{0}, Qty{1});

    std::uint8_t bytes[sizeof(NewOrderMsg)]{};
    std::memcpy(bytes, &msg, sizeof(msg));

    LOG_KV("byte[0] version", static_cast<int>(bytes[0]));
    LOG_KV("byte[1] type", static_cast<int>(bytes[1]));
    LOG_KV("byte[2] length lo", static_cast<int>(bytes[2]));

    REQUIRE(bytes[0] == 1);
    REQUIRE(bytes[1] == static_cast<std::uint8_t>(MsgType::NewOrder));
    REQUIRE(bytes[2] == sizeof(NewOrderMsg));
}

TEST_CASE("price and qty are distinct types", "[types]") {
    LOG_TEST("price and qty are distinct types");
    Price p{100};
    Qty   q{100};
    LOG_KV("price.value", p.value);
    LOG_KV("qty.value", q.value);
    REQUIRE(p.value == q.value);
}
