#pragma once

// matching engine — turns incoming orders into trades
//
// owns an order book. when an order arrives:
//   1. cross it against the opposite side while prices overlap
//   2. emit a Trade for every fill
//   3. limit order leftover rests on the book; market leftover is dropped
//
// price-time priority: always hit the best price first, oldest order first.
// execution price is the resting (maker) order's price.

#include <vector>

#include "exchange/order.hpp"
#include "exchange/order_book.hpp"
#include "exchange/trade.hpp"
#include "exchange/types.hpp"

namespace exchange {

class MatchingEngine {
public:
    // submit an order, returns the trades it generated (may be empty)
    std::vector<Trade> submit(Order incoming);

    // expose the book read-only for queries / tests
    const OrderBook& book() const { return book_; }

private:
    OrderBook book_;
};

}  // namespace exchange
