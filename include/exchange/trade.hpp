#pragma once

// a trade — the record produced when two orders match
//
// the incoming order is the "taker" (it takes liquidity).
// the resting order it hits is the "maker" (it made liquidity earlier).
// execution price = the maker's price (it was there first — price-time priority).

#include <type_traits>

#include "exchange/time/timestamp.hpp"
#include "exchange/types.hpp"

namespace exchange {

struct Trade {
    OrderId   taker_id{};  // the aggressor — the incoming order
    OrderId   maker_id{};  // the resting order that got filled
    Price     price{};     // executed at the maker's resting price
    Qty       qty{};       // how much filled in this match
    Timestamp ts{};        // when the match happened
};

// must stay plain-old-data — trades get published over the wire later
static_assert(std::is_trivially_copyable_v<Trade>);

}  // namespace exchange
