// entry point — placeholder until we wire up feed + matcher processes
#include <cstdio>

#include "exchange/protocol/messages.hpp"
#include "exchange/types.hpp"

int main() {
    using namespace exchange;
    using namespace exchange::protocol;

    // smoke test: domain types + wire structs compile and link
    const Price px{10025};
    const Qty   qty{500};

    const auto msg = make_new_order(OrderId{42}, Side::Buy, OrderType::Limit, px, qty);

    std::printf("latency-exchange v0.1.0  price=%lld qty=%lld wire_size=%zu\n",
                static_cast<long long>(px.value),
                static_cast<long long>(qty.value),
                sizeof(msg));
    return 0;
}
