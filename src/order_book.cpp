// order book implementation
//
// data model (see order_book.hpp):
//   bids_/asks_ : map<Price, list<Order>>  — price levels, time priority within
//   index_      : order id -> Location     — O(1) cancel/modify
//
// milestone 1: add() + best bid/ask + level counts
// cancel/modify land in the next milestones

#include <iterator>

#include "exchange/order_book.hpp"

namespace exchange {

bool OrderBook::add(const Order& order) {
    // only limit orders rest on the book — market orders take liquidity (matcher's job)
    if (order.type != OrderType::Limit) {
        return false;
    }

    // a resting order must have something left to fill
    if (order.remaining.value <= 0) {
        return false;
    }

    // ids are unique — refuse duplicates so the index stays consistent
    if (index_.find(order.id.value) != index_.end()) {
        return false;
    }

    // find or create the price level on the right side
    SideMap& book = side_map(order.side);
    Level& level  = book[order.price];

    // append to the back = newest at a price level (fifo time priority)
    level.push_back(order);
    Level::iterator it = std::prev(level.end());

    // remember where it lives for fast cancel/modify later
    index_.emplace(order.id.value, Location{order.side, order.price, it});
    return true;
}

bool OrderBook::cancel(OrderId) {
    return false;  // milestone 2
}

bool OrderBook::modify(OrderId, Price, Qty) {
    return false;  // milestone 3
}

std::optional<Price> OrderBook::best_bid() const {
    if (bids_.empty()) {
        return std::nullopt;
    }
    // map is ascending — highest bid is the last key
    return bids_.rbegin()->first;
}

std::optional<Price> OrderBook::best_ask() const {
    if (asks_.empty()) {
        return std::nullopt;
    }
    // lowest ask is the first key
    return asks_.begin()->first;
}

std::size_t OrderBook::bid_levels() const {
    return bids_.size();
}

std::size_t OrderBook::ask_levels() const {
    return asks_.size();
}

}  // namespace exchange
