#include <cstring>

#include <catch2/catch_test_macros.hpp>

#include "exchange/protocol/messages.hpp"
#include "exchange/protocol/validate.hpp"
#include "exchange/types.hpp"

using namespace exchange;
using namespace exchange::protocol;

TEST_CASE("validate_message accepts valid new order", "[validate]") {
    const auto msg = make_new_order(OrderId{1}, Side::Buy, OrderType::Limit,
                                    Price{100}, Qty{10});

    std::uint8_t buf[sizeof(NewOrderMsg)]{};
    std::memcpy(buf, &msg, sizeof(msg));

    REQUIRE(validate_message(buf) == ParseError::Ok);

    const auto* parsed = as_message<NewOrderMsg>(buf);
    REQUIRE(parsed != nullptr);
    REQUIRE(parsed->order_id.value == 1);
}

TEST_CASE("validate_message rejects truncated buffer", "[validate]") {
    std::uint8_t buf[2]{1, 1};
    REQUIRE(validate_message(buf) == ParseError::BufferTooSmall);
}

TEST_CASE("validate_message rejects bad version", "[validate]") {
    const auto msg = make_cancel(OrderId{5});
    std::uint8_t buf[sizeof(CancelMsg)]{};
    std::memcpy(buf, &msg, sizeof(msg));
    buf[0] = 99;

    REQUIRE(validate_message(buf) == ParseError::BadVersion);
}

TEST_CASE("validate_message rejects length mismatch", "[validate]") {
    const auto msg = make_cancel(OrderId{5});
    std::uint8_t buf[sizeof(CancelMsg)]{};
    std::memcpy(buf, &msg, sizeof(buf));
    buf[2] = 0;  // corrupt length lo byte

    REQUIRE(validate_message(buf) == ParseError::LengthMismatch);
}
