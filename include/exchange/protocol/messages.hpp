#pragma once

// internal wire format v0 — fixed-size structs for udp replay and fpga parsing later
// not itch/cme yet, just our own binary layout

#include <cstdint>
#include <cstring>

#include "exchange/types.hpp"

namespace exchange::protocol {

// little-endian on wire (matches x86/linux host, no byte swap needed locally)
enum class MsgType : std::uint8_t {
    NewOrder  = 1,
    Cancel    = 2,
    Modify    = 3,
    Heartbeat = 255,
};

#pragma pack(push, 1)

// every message starts with this — parser reads header first, then branches on type
struct MessageHeader {
    std::uint8_t  version{1};
    MsgType       type{};
    std::uint16_t length{};  // full message size in bytes, header included
};

struct NewOrderMsg {
    MessageHeader hdr{};
    OrderId       order_id{};
    Side          side{};
    OrderType     order_type{};
    std::uint8_t  reserved{0};  // pad to keep price 8-byte aligned on most targets
    Price         price{};
    Qty           qty{};
};

struct CancelMsg {
    MessageHeader hdr{};
    OrderId       order_id{};
};

struct ModifyMsg {
    MessageHeader hdr{};
    OrderId       order_id{};
    Price         new_price{};
    Qty           new_qty{};
};

#pragma pack(pop)

// factory helpers — keeps header fields consistent at call sites
inline NewOrderMsg make_new_order(OrderId id, Side side, OrderType ot,
                                  Price px, Qty qty) {
    NewOrderMsg m{};
    m.hdr.version = 1;
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
    m.hdr.version = 1;
    m.hdr.type    = MsgType::Cancel;
    m.hdr.length  = static_cast<std::uint16_t>(sizeof(CancelMsg));
    m.order_id    = id;
    return m;
}

inline ModifyMsg make_modify(OrderId id, Price px, Qty qty) {
    ModifyMsg m{};
    m.hdr.version = 1;
    m.hdr.type    = MsgType::Modify;
    m.hdr.length  = static_cast<std::uint16_t>(sizeof(ModifyMsg));
    m.order_id    = id;
    m.new_price   = px;
    m.new_qty     = qty;
    return m;
}

// if these fail, compiler added padding — breaks wire compat and fpga struct layout
static_assert(sizeof(MessageHeader) == 4);
static_assert(sizeof(NewOrderMsg) == 31);
static_assert(sizeof(CancelMsg) == 12);
static_assert(sizeof(ModifyMsg) == 28);

static_assert(std::is_trivially_copyable_v<NewOrderMsg>);
static_assert(std::is_trivially_copyable_v<CancelMsg>);
static_assert(std::is_trivially_copyable_v<ModifyMsg>);

}  // namespace exchange::protocol
