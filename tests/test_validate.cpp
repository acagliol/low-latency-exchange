// tests for validate_message / as_message
// these run on every build — if wire layout changes, fix messages.hpp first

#include <cstring>

#include <catch2/catch_test_macros.hpp>

#include "exchange/protocol/messages.hpp"
#include "exchange/protocol/validate.hpp"
#include "exchange/types.hpp"
#include "test_log.hpp"

using namespace exchange;
using namespace exchange::protocol;

TEST_CASE("validate_message accepts valid new order", "[validate]") {
    LOG_TEST("validate accepts valid new order");

    // build a real wire message, copy to a byte buffer (simulates udp recv)
    const auto msg = make_new_order(OrderId{1}, Side::Buy, OrderType::Limit,
                                    Price{100}, Qty{10});

    std::uint8_t buf[sizeof(NewOrderMsg)]{};
    std::memcpy(buf, &msg, sizeof(msg));
    LOG_KV("buffer bytes", sizeof(buf));

    const auto result = validate_message(buf);
    LOG_KV("validate result (0=Ok)", static_cast<int>(result));
    REQUIRE(result == ParseError::Ok);

    // safe cast should work and fields should match
    const auto* parsed = as_message<NewOrderMsg>(buf);
    REQUIRE(parsed != nullptr);
    LOG_KV("parsed order_id", parsed->order_id.value);
    REQUIRE(parsed->order_id.value == 1);
}

TEST_CASE("validate_message rejects truncated buffer", "[validate]") {
    LOG_TEST("validate rejects truncated buffer");

    // only 2 bytes — can't even fit the 4-byte header
    std::uint8_t buf[2]{1, 1};
    LOG_KV("buffer bytes", sizeof(buf));

    const auto result = validate_message(buf);
    LOG_KV("validate result (1=BufferTooSmall)", static_cast<int>(result));
    REQUIRE(result == ParseError::BufferTooSmall);
}

TEST_CASE("validate_message rejects bad version", "[validate]") {
    LOG_TEST("validate rejects bad version");

    const auto msg = make_cancel(OrderId{5});
    std::uint8_t buf[sizeof(CancelMsg)]{};
    std::memcpy(buf, &msg, sizeof(msg));

    // corrupt version byte — simulates talking to wrong protocol version
    LOG_KV("version before corruption", static_cast<int>(buf[0]));
    buf[0] = 99;
    LOG_KV("version after corruption", static_cast<int>(buf[0]));

    const auto result = validate_message(buf);
    LOG_KV("validate result (2=BadVersion)", static_cast<int>(result));
    REQUIRE(result == ParseError::BadVersion);
}

TEST_CASE("validate_message rejects length mismatch", "[validate]") {
    LOG_TEST("validate rejects length mismatch");

    const auto msg = make_cancel(OrderId{5});
    std::uint8_t buf[sizeof(CancelMsg)]{};
    std::memcpy(buf, &msg, sizeof(buf));

    // corrupt length lo byte — header claims wrong size
    LOG_KV("length lo byte before", static_cast<int>(buf[2]));
    buf[2] = 0;
    LOG_KV("length lo byte after", static_cast<int>(buf[2]));

    const auto result = validate_message(buf);
    LOG_KV("validate result (4=LengthMismatch)", static_cast<int>(result));
    REQUIRE(result == ParseError::LengthMismatch);
}
