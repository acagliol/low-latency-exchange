#pragma once

// safe parsing — check raw bytes before casting to wire structs
// feed handler calls this on every udp packet before touching memory as a message

#include <cstdint>
#include <cstring>
#include <span>

#include "exchange/protocol/messages.hpp"

namespace exchange::protocol {

enum class ParseError : std::uint8_t {
    Ok             = 0,
    BufferTooSmall = 1,
    BadVersion     = 2,
    UnknownType    = 3,
    LengthMismatch = 4,
};

// expected wire size for a known message type
inline std::uint16_t expected_length(MsgType type) {
    switch (type) {
    case MsgType::NewOrder:  return static_cast<std::uint16_t>(sizeof(NewOrderMsg));
    case MsgType::Cancel:    return static_cast<std::uint16_t>(sizeof(CancelMsg));
    case MsgType::Modify:    return static_cast<std::uint16_t>(sizeof(ModifyMsg));
  // heartbeat has no body struct yet — minimum is header only
    case MsgType::Heartbeat: return static_cast<std::uint16_t>(sizeof(MessageHeader));
    }
    return 0;
}

// validate header fields and that buffer is big enough for claimed length
inline ParseError validate_message(std::span<const std::uint8_t> data) {
    if (data.size() < sizeof(MessageHeader)) {
        return ParseError::BufferTooSmall;
    }

    MessageHeader hdr{};
    std::memcpy(&hdr, data.data(), sizeof(hdr));

    if (hdr.version != kProtocolVersion) {
        return ParseError::BadVersion;
    }

    const std::uint16_t expected = expected_length(hdr.type);
    if (expected == 0) {
        return ParseError::UnknownType;
    }

    if (hdr.length != expected) {
        return ParseError::LengthMismatch;
    }

    if (data.size() < hdr.length) {
        return ParseError::BufferTooSmall;
    }

    return ParseError::Ok;
}

// after validate_message returns Ok, cast bytes to a concrete message type
template <typename Msg>
inline const Msg* as_message(std::span<const std::uint8_t> data) {
    if (validate_message(data) != ParseError::Ok) {
        return nullptr;
    }
    if (data.size() < sizeof(Msg)) {
        return nullptr;
    }
    return reinterpret_cast<const Msg*>(data.data());
}

}  // namespace exchange::protocol
