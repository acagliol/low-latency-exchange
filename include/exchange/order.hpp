#pragma once

// in-memory order — what the order book stores after parsing a wire message
//
// difference from wire format (NewOrderMsg):
//   - wire msg is packed bytes for network
//   - Order is our runtime struct with extra fields (remaining, created_at)
//   - Order is NOT packed — compiler can align for cache performance
//
// price-time priority (day 3):
//   - orders at the same price level are sorted by created_at (earliest first)
//   - that's why we carry a timestamp on every order

#include <type_traits>

#include "exchange/time/timestamp.hpp"
#include "exchange/types.hpp"

namespace exchange {

struct Order {
    OrderId     id{};
    Side        side{};        // buy or sell
    OrderType   type{};        // limit (rests) or market (takes)
    Price       price{};       // limit price in ticks — meaningless for market orders
    Qty         qty{};         // original quantity when order was placed
    Qty         remaining{};   // how much is still unfilled — starts == qty
    Timestamp   created_at{};  // when we accepted the order — for time priority

    // factory — builds a fresh resting order from validated wire fields
    // caller is responsible for running validate_message first on the wire bytes
    static Order from_new_order(OrderId id, Side side, OrderType type,
                                Price px, Qty qty, Timestamp ts) {
        Order o{};
        o.id         = id;
        o.side       = side;
        o.type       = type;
        o.price      = px;
        o.qty        = qty;
        o.remaining  = qty;  // nothing filled yet
        o.created_at = ts;
        return o;
    }
};

// must stay plain-old-data — book will store these in vectors / pools
static_assert(std::is_trivially_copyable_v<Order>);

}  // namespace exchange
