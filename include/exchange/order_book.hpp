#pragma once

// limit order book api — implementation lands day 3
// price-time priority: best price first, then earliest order at that price

#include <optional>

#include "exchange/order.hpp"
#include "exchange/types.hpp"

namespace exchange {

class OrderBook {
public:
    // add a resting limit order to the book
    bool add(const Order& order);

    // remove by id — O(1) lookup planned via side index
    bool cancel(OrderId id);

    // change price/qty on existing order
    bool modify(OrderId id, Price new_price, Qty new_qty);

    // best bid = highest buy price, best ask = lowest sell price
    std::optional<Price> best_bid() const;
    std::optional<Price> best_ask() const;

    std::size_t bid_levels() const;
    std::size_t ask_levels() const;
};

}  // namespace exchange
