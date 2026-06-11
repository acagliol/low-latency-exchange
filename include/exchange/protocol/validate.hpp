#pragma once

// safe parsing — check raw bytes BEFORE casting to wire structs
//
// why this exists:
//   udp packets can be truncated, corrupted, or from a wrong protocol version.
//   casting bad bytes directly to NewOrderMsg* is undefined behavior.
//   feed handler will call validate_message() on every inbound packet.
//
// typical flow:
//   1. receive bytes into a buffer
//   2. validate_message(buf) == Ok
//   3. as_message<NewOrderMsg>(buf) to read fields
//   4. build an in-memory Order and pass to the book

#include <cstdint>
#include <cstring>
#include <span>

#include "exchange/protocol/messages.hpp"

namespace exchange::protocol {

// why we return an enum instead of bool — caller can log/reject with a reason
enum class ParseError : std::uint8_t {
    Ok             = 0,  // message looks valid
    BufferTooSmall = 1,  // not enough bytes for header or claimed length
    BadVersion     = 2,  // hdr.version != kProtocolVersion
    UnknownType    = 3,  // msg type byte not in our enum
    LengthMismatch = 4,  // hdr.length doesn't match sizeof() for that type
};

// lookup table logic — given a type, how many bytes should the full message be?
// returns 0 for unknown types (caller treats as error)
inline std::uint16_t expected_length(MsgType type) {
    switch (type) {
    case MsgType::NewOrder:
        return static_cast<std::uint16_t>(sizeof(NewOrderMsg));   // 31
    case MsgType::Cancel:
        return static_cast<std::uint16_t>(sizeof(CancelMsg));     // 12
    case MsgType::Modify:
        return static_cast<std::uint16_t>(sizeof(ModifyMsg));     // 28
    // heartbeat has no body struct yet — header-only is valid for now
    case MsgType::Heartbeat:
        return static_cast<std::uint16_t>(sizeof(MessageHeader)); // 4
    }
    return 0;
}

// main entry point — call this before any cast
//
// checks (in order):
//   1. buffer >= 4 bytes (fits MessageHeader)
//   2. version field matches kProtocolVersion
//   3. type is known (expected_length != 0)
//   4. hdr.length matches expected size for that type
//   5. buffer actually contains hdr.length bytes (not just a lying header)
inline ParseError validate_message(std::span<const std::uint8_t> data) {
    // step 1 — can't even read a header
    if (data.size() < sizeof(MessageHeader)) {
        return ParseError::BufferTooSmall;
    }

    // copy header out — avoids unaligned cast on weird buffers
    MessageHeader hdr{};
    std::memcpy(&hdr, data.data(), sizeof(hdr));

    // step 2 — wrong protocol version means sender and receiver disagree on layout
    if (hdr.version != kProtocolVersion) {
        return ParseError::BadVersion;
    }

    // step 3 + 4 — type must be known and length must match our struct sizes
    const std::uint16_t expected = expected_length(hdr.type);
    if (expected == 0) {
        return ParseError::UnknownType;
    }

    if (hdr.length != expected) {
        return ParseError::LengthMismatch;
    }

    // step 5 — header says message is N bytes but buffer might be shorter
    if (data.size() < hdr.length) {
        return ParseError::BufferTooSmall;
    }

    return ParseError::Ok;
}

// safe cast helper — returns nullptr on any validation failure
// only use after validate_message, or just call this directly (it re-validates)
//
// reinterpret_cast is ok here because:
//   - we validated version, type, and length
//   - structs are trivially copyable / standard layout
//   - x86 host is little-endian matching wire format
template <typename Msg>
inline const Msg* as_message(std::span<const std::uint8_t> data) {
    if (validate_message(data) != ParseError::Ok) {
        return nullptr;
    }
    // extra guard — span might be valid header but wrong concrete type size
    if (data.size() < sizeof(Msg)) {
        return nullptr;
    }
    return reinterpret_cast<const Msg*>(data.data());
}

}  // namespace exchange::protocol
