#pragma once

// in-memory order — what the book stores after parsing a wire message

#include <type_traits>

#include "exchange/time/timestamp.hpp"
#include "exchange/types.hpp"

namespace exchange {

struct Order {
    OrderId     id{};
    Side        side{};
    OrderType   type{};
    Price       price{};
    Qty         qty{};           // original quantity
    Qty         remaining{};     // unfilled quantity
    Timestamp   created_at{};

    // build from a validated new-order wire message
    static Order from_new_order(OrderId id, Side side, OrderType type,
                                Price px, Qty qty, Timestamp ts) {
        Order o{};
        o.id         = id;
        o.side       = side;
        o.type       = type;
        o.price      = px;
        o.qty        = qty;
        o.remaining  = qty;
        o.created_at = ts;
        return o;
    }
};

static_assert(std::is_trivially_copyable_v<Order>);

}  // namespace exchange
