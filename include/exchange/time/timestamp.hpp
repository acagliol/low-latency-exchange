#pragma once

// monotonic timestamps for latency measurement
//
// we use steady_clock, NOT system_clock:
//   - steady_clock never goes backward (ntp adjustments won't break deltas)
//   - good for "how long between packet receive and match?"
//   - bad for wall-clock display — add system_clock later if needed
//
// usage later:
//   auto t0 = now();          // packet arrives
//   ... process order ...
//   auto t1 = now();          // order on book
//   auto latency = t1.elapsed_ns(t0);

#include <chrono>
#include <cstdint>

namespace exchange {

struct Timestamp {
    std::int64_t ns{};  // nanoseconds since steady_clock epoch (not unix time)

    constexpr Timestamp() = default;
    constexpr explicit Timestamp(std::int64_t nanos) : ns(nanos) {}

    // how many ns between two events
    // positive if *this is after other, negative if before
    constexpr std::int64_t elapsed_ns(Timestamp other) const {
        return ns - other.ns;
    }

    // c++20 spaceship — lets us sort orders by time at the same price level
    auto operator<=>(const Timestamp&) const = default;
};

// snapshot the clock right now — inline so no function call overhead in hot path
inline Timestamp now() {
    const auto t = std::chrono::steady_clock::now().time_since_epoch();
    return Timestamp{std::chrono::duration_cast<std::chrono::nanoseconds>(t).count()};
}

}  // namespace exchange
