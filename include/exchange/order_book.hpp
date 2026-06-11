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
#include <cstdint>
#include <list>
#include <map>
#include <optional>
#include <unordered_map>

#include "exchange/order.hpp"
#include "exchange/types.hpp"

namespace exchange {

class OrderBook {
public:
    // insert a resting limit order
    // returns false if order is invalid (market order, zero qty, duplicate id, etc.)
    bool add(const Order& order);

    // remove an order by id
    // returns false if id not found
    // O(1) via order id index
    bool cancel(OrderId id);

    // change price and/or qty on an existing order
    // cancel-replace semantics (remove from old level, insert at new)
    bool modify(OrderId id, Price new_price, Qty new_qty);

    // best bid = highest buy price on the book (nullopt if no bids)
    std::optional<Price> best_bid() const;

    // best ask = lowest sell price on the book (nullopt if no asks)
    std::optional<Price> best_ask() const;

    // how many distinct price levels exist on each side
    // e.g. bids at 100 and 99 = 2 bid levels
    std::size_t bid_levels() const;
    std::size_t ask_levels() const;

    // total resting quantity at one price level (0 if no such level)
    // this is "depth" — how much size sits at a given price
    Qty qty_at_price(Side side, Price price) const;

    // total number of live orders across the whole book
    std::size_t order_count() const;

private:
    // one price level — FIFO list of orders = time priority (earliest at front)
    // std::list so we can erase any order in O(1) without moving the others
    using Level = std::list<Order>;

    // a whole side, keyed by price and kept sorted by the map
    //   bids: best = highest price = rbegin()
    //   asks: best = lowest price  = begin()
    using SideMap = std::map<Price, Level>;

    // where an order lives, so cancel/modify can jump straight to it
    struct Location {
        Side            side;
        Price           price;
        Level::iterator it;  // stays valid because Level is a std::list
    };

    SideMap bids_;
    SideMap asks_;

    // order id (raw uint64) -> location, for O(1) lookup
    std::unordered_map<std::uint64_t, Location> index_;

    // pick the right side map for a buy/sell
    SideMap& side_map(Side side) { return side == Side::Buy ? bids_ : asks_; }
    const SideMap& side_map(Side side) const { return side == Side::Buy ? bids_ : asks_; }
};

}  // namespace exchange
