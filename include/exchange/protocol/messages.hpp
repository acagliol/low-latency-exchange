#pragma once

// internal wire format v0 — fixed-size structs for udp replay and fpga parsing later
// not itch/cme yet, just our own binary layout
//
// design rules:
//   - every message is a plain struct (no virtuals, no heap)
//   - sizes are fixed and checked at compile time
//   - parser flow: read header -> check length -> cast body by type

#include <cstdint>
#include <cstring>

#include "exchange/types.hpp"

namespace exchange::protocol {

// protocol version — bump this when you break wire layout (never silently)
inline constexpr std::uint8_t kProtocolVersion = 1;

// little-endian on wire (matches x86/linux host, no byte swap needed locally)
// heartbeat uses 255 so it's obvious in a hex dump and won't collide with order msgs
enum class MsgType : std::uint8_t {
    NewOrder  = 1,
    Cancel    = 2,
    Modify    = 3,
    Heartbeat = 255,
};

// pack(1) = no compiler padding between fields
// tradeoff: unaligned loads on some arm chips — fine on x86_64, our primary target
#pragma pack(push, 1)

// every message starts with this — feed handler reads 4 bytes, then knows what follows
//
// wire layout (4 bytes total):
//   offset 0: version  (u8)
//   offset 1: type     (u8)  — see MsgType
//   offset 2: length   (u16) — entire message size, header included
struct MessageHeader {
    std::uint8_t  version{1};
    MsgType       type{};
    std::uint16_t length{};  // must match sizeof() for that message type
};

// place a new order on the book (or cross if market)
// wire size: 31 bytes (4 header + 27 payload)
struct NewOrderMsg {
    MessageHeader hdr{};
    OrderId       order_id{};    // unique per session — used for cancel/modify lookup
    Side          side{};
    OrderType     order_type{};
    std::uint8_t  reserved{0};   // pad byte — keeps price 8-byte aligned on most targets
    Price         price{};       // ignored for market orders (set 0)
    Qty           qty{};
};

// pull an order off the book by id
// wire size: 12 bytes
struct CancelMsg {
    MessageHeader hdr{};
    OrderId       order_id{};
};

// change price/qty on an existing order (cancel-replace semantics later)
// wire size: 28 bytes
struct ModifyMsg {
    MessageHeader hdr{};
    OrderId       order_id{};
    Price         new_price{};
    Qty           new_qty{};
};

#pragma pack(pop)

// --- factories ---
// use these instead of hand-filling hdr — easy to forget length or version

inline NewOrderMsg make_new_order(OrderId id, Side side, OrderType ot,
                                  Price px, Qty qty) {
    NewOrderMsg m{};
    m.hdr.version = kProtocolVersion;
    m.hdr.type    = MsgType::NewOrder;
    m.hdr.length  = static_cast<std::uint16_t>(sizeof(NewOrderMsg));
    m.order_id    = id;
    m.side        = side;
    m.order_type  = ot;
    m.price       = px;
    m.qty         = qty;
    return m;
}

inline CancelMsg make_cancel(OrderId id) {
    CancelMsg m{};
    m.hdr.version = kProtocolVersion;
    m.hdr.type    = MsgType::Cancel;
    m.hdr.length  = static_cast<std::uint16_t>(sizeof(CancelMsg));
    m.order_id    = id;
    return m;
}

inline ModifyMsg make_modify(OrderId id, Price px, Qty qty) {
    ModifyMsg m{};
    m.hdr.version = kProtocolVersion;
    m.hdr.type    = MsgType::Modify;
    m.hdr.length  = static_cast<std::uint16_t>(sizeof(ModifyMsg));
    m.order_id    = id;
    m.new_price   = px;
    m.new_qty     = qty;
    return m;
}

// --- compile-time wire size guards ---
// if any of these fail, the compiler inserted padding — do not change the expected
// sizes without bumping kProtocolVersion and updating tests + fpga structs

static_assert(sizeof(MessageHeader) == 4);
static_assert(sizeof(NewOrderMsg) == 31);
static_assert(sizeof(CancelMsg) == 12);
static_assert(sizeof(ModifyMsg) == 28);

// max single message fits in u16 length field (65k) — we're nowhere near that
static_assert(sizeof(NewOrderMsg) <= 65535);

// safe to memcpy to/from udp buffer, shm ring, or dma region
static_assert(std::is_trivially_copyable_v<NewOrderMsg>);
static_assert(std::is_trivially_copyable_v<CancelMsg>);
static_assert(std::is_trivially_copyable_v<ModifyMsg>);

}  // namespace exchange::protocol
