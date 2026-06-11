// order book implementation
//
// day 2: stub only — every method is a no-op so tests compile
// day 3: replace these with real price levels + id lookup
//
// why a .cpp and not header-only:
//   - book internals will get big (maps, pools, indices)
//   - keep the public api clean in order_book.hpp
//   - only recompile this file when book logic changes

#include "exchange/order_book.hpp"

namespace exchange {

// stub — always fails, day 3 inserts into bid/ask trees
bool OrderBook::add(const Order&) {
    return false;
}

// stub — always fails, day 3 removes from id index + price level
bool OrderBook::cancel(OrderId) {
    return false;
}

// stub — always fails, day 3 does cancel-replace
bool OrderBook::modify(OrderId, Price, Qty) {
    return false;
}

// stub — no bids yet
std::optional<Price> OrderBook::best_bid() const {
    return std::nullopt;
}

// stub — no asks yet
std::optional<Price> OrderBook::best_ask() const {
    return std::nullopt;
}

// stub
std::size_t OrderBook::bid_levels() const {
    return 0;
}

// stub
std::size_t OrderBook::ask_levels() const {
    return 0;
}

}  // namespace exchange
