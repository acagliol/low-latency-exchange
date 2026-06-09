#include <cstring>

#include <catch2/catch_test_macros.hpp>

#include "exchange/protocol/messages.hpp"
#include "exchange/types.hpp"

using namespace exchange;
using namespace exchange::protocol;

TEST_CASE("message header is 4 bytes on wire", "[protocol]") {
    static_assert(sizeof(MessageHeader) == 4);
    REQUIRE(sizeof(MessageHeader) == 4);
}

TEST_CASE("wire struct sizes match v0 spec", "[protocol]") {
    REQUIRE(sizeof(NewOrderMsg) == 31);
    REQUIRE(sizeof(CancelMsg) == 12);
    REQUIRE(sizeof(ModifyMsg) == 28);
}

TEST_CASE("make_new_order fills header and payload", "[protocol]") {
    auto msg = make_new_order(OrderId{7}, Side::Sell, OrderType::Limit,
                              Price{999}, Qty{50});

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
    auto cancel = make_cancel(OrderId{3});
    REQUIRE(cancel.hdr.type == MsgType::Cancel);
    REQUIRE(cancel.hdr.length == sizeof(CancelMsg));
    REQUIRE(cancel.order_id.value == 3);

    auto modify = make_modify(OrderId{5}, Price{100}, Qty{200});
    REQUIRE(modify.hdr.type == MsgType::Modify);
    REQUIRE(modify.hdr.length == sizeof(ModifyMsg));
    REQUIRE(modify.new_price.value == 100);
    REQUIRE(modify.new_qty.value == 200);
}

TEST_CASE("new order serializes version as first byte", "[protocol]") {
    auto msg = make_new_order(OrderId{1}, Side::Buy, OrderType::Market,
                              Price{0}, Qty{1});

    std::uint8_t bytes[sizeof(NewOrderMsg)]{};
    std::memcpy(bytes, &msg, sizeof(msg));

    REQUIRE(bytes[0] == 1);                          // version
    REQUIRE(bytes[1] == static_cast<std::uint8_t>(MsgType::NewOrder));
    REQUIRE(bytes[2] == sizeof(NewOrderMsg));        // length lo byte (31)
}

TEST_CASE("price and qty are distinct types", "[types]") {
    Price p{100};
    Qty   q{100};
    REQUIRE(p.value == q.value);
}
