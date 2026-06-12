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

bool OrderBook::cancel(OrderId id) {
    // O(1) — the index tells us exactly where the order lives
    auto idx_it = index_.find(id.value);
    if (idx_it == index_.end()) {
        return false;  // unknown id (already filled, never existed, etc.)
    }

    const Location& loc = idx_it->second;
    SideMap& book = side_map(loc.side);

    auto level_it = book.find(loc.price);
    if (level_it != book.end()) {
        // erase the exact order node — O(1) on a std::list, others stay valid
        level_it->second.erase(loc.it);

        // if that was the last order at this price, drop the empty level
        if (level_it->second.empty()) {
            book.erase(level_it);
        }
    }

    index_.erase(idx_it);
    return true;
}

bool OrderBook::modify(OrderId id, Price new_price, Qty new_qty) {
    // cancel-replace: a price/size change loses time priority, so we remove
    // the old order and re-add a fresh one at the back of the new level
    auto idx_it = index_.find(id.value);
    if (idx_it == index_.end()) {
        return false;
    }

    if (new_qty.value <= 0) {
        return false;  // a modify to zero qty should be a cancel instead
    }

    // grab a copy of the existing order before we erase it
    const Location& loc = idx_it->second;
    Order updated = *loc.it;
    updated.price     = new_price;
    updated.qty       = new_qty;
    updated.remaining = new_qty;

    // remove the old, insert the new (keeps index + levels consistent)
    cancel(id);
    return add(updated);
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

Qty OrderBook::qty_at_price(Side side, Price price) const {
    const SideMap& book = side_map(side);
    auto level_it = book.find(price);
    if (level_it == book.end()) {
        return Qty{0};
    }

    // sum remaining across every order resting at this price
    std::int64_t total = 0;
    for (const Order& o : level_it->second) {
        total += o.remaining.value;
    }
    return Qty{total};
}

std::size_t OrderBook::order_count() const {
    return index_.size();
}

bool OrderBook::has_side(Side side) const {
    return !side_map(side).empty();
}

Price OrderBook::best_price(Side side) const {
    const SideMap& book = side_map(side);
    // buy: highest price (last key). sell: lowest price (first key).
    return side == Side::Buy ? book.rbegin()->first : book.begin()->first;
}

OrderId OrderBook::front_id(Side side) const {
    const Price p = best_price(side);
    return side_map(side).at(p).front().id;
}

Qty OrderBook::front_remaining(Side side) const {
    const Price p = best_price(side);
    return side_map(side).at(p).front().remaining;
}

void OrderBook::fill_front(Side side, Qty filled) {
    SideMap& book = side_map(side);
    const Price p = best_price(side);
    auto level_it = book.find(p);
    Level& level  = level_it->second;

    Order& front = level.front();
    front.remaining.value -= filled.value;

    // fully filled — evict from book and index
    if (front.remaining.value <= 0) {
        index_.erase(front.id.value);
        level.pop_front();
        if (level.empty()) {
            book.erase(level_it);
        }
    }
}

}  // namespace exchange
