#pragma once

// core domain types — every subsystem includes this
// feed handler, order book, matcher, publisher all share these defs

#include <cstdint>
#include <type_traits>

namespace exchange {

// uint8_t keeps wire messages small (milestone 4)
// enum class blocks implicit int conversion mistakes
enum class Side : std::uint8_t { Buy = 0, Sell = 1 };

enum class OrderType : std::uint8_t {
    Limit  = 0,  // rests on book until filled or canceled
    Market = 1,  // crosses spread immediately, no resting price
};

// same idea as Haskell newtypes / rust newtype pattern
// Tag is a phantom type — never stored, only used for compile-time identity
template <typename Tag, typename T>
struct StrongTypedef {
    T value{};

    constexpr StrongTypedef() = default;
    constexpr explicit StrongTypedef(T v) : value(v) {}

    // explicit so Price wont silently become int64_t in arithmetic
    constexpr explicit operator T() const { return value; }

    // c++20 spaceship — gives ==, !=, <, >, <=, >= for free
    auto operator<=>(const StrongTypedef&) const = default;
};

// empty tag structs — zero runtime cost, distinct types at compile time
struct PriceTag {};
struct QtyTag {};
struct OrderIdTag {};

// price in ticks: multiply by tick_size to get dollars
// e.g. tick_size=0.01 → Price{10025} == $100.25
using Price   = StrongTypedef<PriceTag, std::int64_t>;
using Qty     = StrongTypedef<QtyTag, std::int64_t>;       // shares / contracts / lots
using OrderId = StrongTypedef<OrderIdTag, std::uint64_t>;  // monotonic, unique per session

// compile-time guard: if any of these fail, something added a pointer or vtable
// we need trivially copyable for memcpy, dma, and packed wire structs
static_assert(std::is_trivially_copyable_v<Price>);
static_assert(std::is_trivially_copyable_v<Qty>);
static_assert(std::is_trivially_copyable_v<OrderId>);

}  // namespace exchange
