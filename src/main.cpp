// entry point — placeholder until we wire up feed + matcher processes
#include <cstdio>

#include "exchange/types.hpp"

int main() {
    using namespace exchange;

    // smoke test: if this builds and runs, milestone 3 is good
    // replace this whole file once real subsystems have their own mains
    const Price px{10025};  // 100.25 dollars at tick_size 0.01
    const Qty   qty{500};   // 500 shares

    // .value because printf needs a plain integer, not our strong type
    std::printf("latency-exchange v0.1.0  price=%lld qty=%lld\n",
                static_cast<long long>(px.value),
                static_cast<long long>(qty.value));
    return 0;
}
