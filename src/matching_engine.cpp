// matching engine implementation
//
// the core loop: keep filling the incoming order against the best opposite
// price until it's done or the prices no longer cross.

#include <algorithm>

#include "exchange/matching_engine.hpp"
#include "exchange/time/timestamp.hpp"

namespace exchange {

namespace {

Side opposite(Side side) {
    return side == Side::Buy ? Side::Sell : Side::Buy;
}

// does an incoming order want to trade at this resting price?
//   buy: willing to pay up to its price, so trade if ask <= price
//   sell: willing to sell down to its price, so trade if bid >= price
//   market: always crosses, price is ignored
bool crosses(const Order& incoming, Price resting) {
    if (incoming.type == OrderType::Market) {
        return true;
    }
    if (incoming.side == Side::Buy) {
        return resting.value <= incoming.price.value;
    }
    return resting.value >= incoming.price.value;
}

}  // namespace

std::vector<Trade> MatchingEngine::submit(Order incoming) {
    std::vector<Trade> trades;
    const Side opp = opposite(incoming.side);

    // fill against the opposite side while there's liquidity and prices cross
    while (incoming.remaining.value > 0 && book_.has_side(opp)) {
        const Price best = book_.best_price(opp);
        if (!crosses(incoming, best)) {
            break;  // no price overlap — stop matching
        }

        // match against the oldest order at the best price (time priority)
        const OrderId maker_id  = book_.front_id(opp);
        const Qty     maker_rem = book_.front_remaining(opp);

        // fill the smaller of the two remaining quantities
        const std::int64_t fill = std::min(incoming.remaining.value, maker_rem.value);

        // trade executes at the maker's resting price
        trades.push_back(Trade{incoming.id, maker_id, best, Qty{fill}, now()});

        book_.fill_front(opp, Qty{fill});
        incoming.remaining.value -= fill;
    }

    // leftover handling:
    //   limit  -> rest the unfilled remainder on the book
    //   market -> discard (no resting price to sit at)
    if (incoming.remaining.value > 0 && incoming.type == OrderType::Limit) {
        book_.add(incoming);
    }

    return trades;
}

}  // namespace exchange
