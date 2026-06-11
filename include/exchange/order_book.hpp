#pragma once

// limit order book — public api
//
// implementation is in order_book.cpp (day 3 fills in the real logic)
// right now it's a stub so we can write tests first (tdd)
//
// price-time priority rules:
//   bids: highest price wins (best bid = max buy price)
//   asks: lowest price wins (best ask = min sell price)
//   at the same price: earliest created_at wins
//
// planned internals (day 3):
//   - std::map<Price, Level> for each side (or flat map for cache)
//   - std::unordered_map<OrderId, location> for O(1) cancel
//   - BBO cached or computed from best level in each map

#include <cstddef>
#include <optional>

#include "exchange/order.hpp"
#include "exchange/types.hpp"

namespace exchange {

class OrderBook {
public:
    // insert a resting limit order
    // returns false if order is invalid (market order, zero qty, duplicate id, etc.)
    // day 3: actually inserts into price level
    bool add(const Order& order);

    // remove an order by id
    // returns false if id not found
    // day 3: O(1) via order id index
    bool cancel(OrderId id);

    // change price and/or qty on an existing order
    // day 3: cancel-replace semantics (remove from old level, insert at new)
    bool modify(OrderId id, Price new_price, Qty new_qty);

    // best bid = highest buy price on the book (nullopt if no bids)
    std::optional<Price> best_bid() const;

    // best ask = lowest sell price on the book (nullopt if no asks)
    std::optional<Price> best_ask() const;

    // how many distinct price levels exist on each side
    // e.g. bids at 100 and 99 = 2 bid levels
    std::size_t bid_levels() const;
    std::size_t ask_levels() const;
};

}  // namespace exchange
