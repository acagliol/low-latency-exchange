// stub implementation — day 3 fills in price levels + id lookup

#include "exchange/order_book.hpp"

namespace exchange {

bool OrderBook::add(const Order&) { return false; }

bool OrderBook::cancel(OrderId) { return false; }

bool OrderBook::modify(OrderId, Price, Qty) { return false; }

std::optional<Price> OrderBook::best_bid() const { return std::nullopt; }

std::optional<Price> OrderBook::best_ask() const { return std::nullopt; }

std::size_t OrderBook::bid_levels() const { return 0; }

std::size_t OrderBook::ask_levels() const { return 0; }

}  // namespace exchange
