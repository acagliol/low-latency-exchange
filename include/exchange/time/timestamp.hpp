#pragma once

// monotonic timestamps for latency measurement
// steady_clock — never goes backward, good for deltas between events

#include <chrono>
#include <cstdint>

namespace exchange {

struct Timestamp {
    std::int64_t ns{};  // nanoseconds since steady_clock epoch

    constexpr Timestamp() = default;
    constexpr explicit Timestamp(std::int64_t nanos) : ns(nanos) {}

    constexpr std::int64_t elapsed_ns(Timestamp other) const {
        return ns - other.ns;
    }

    auto operator<=>(const Timestamp&) const = default;
};

// capture now — call on packet receive, order submit, match, etc.
inline Timestamp now() {
    const auto t = std::chrono::steady_clock::now().time_since_epoch();
    return Timestamp{std::chrono::duration_cast<std::chrono::nanoseconds>(t).count()};
}

}  // namespace exchange
