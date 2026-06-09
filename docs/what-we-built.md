# what we built so far (day 1)

plain-english overview of the project state after the first five commits.

---

## the simple version

we are building a **fake stock exchange in c++** — the same kind of software that sits between traders and the market, matching buy and sell orders. eventually we want it to be **really fast** (low latency) and optionally speed things up with an **fpga** (a chip you can program to process network packets in hardware).

**right now we do not have a working exchange yet.** what we have is the **foundation**:

1. a clean project folder and git repo
2. a build system (cmake) that compiles c++20 code
3. basic **definitions** for trading concepts (price, quantity, buy/sell, etc.)
4. a **binary message format** — the exact byte layout messages will use on the network
5. **automated tests** that prove the message layout is correct

think of it like building a house: we poured the foundation and framed the walls. no plumbing (order book), no electricity (matching engine), no front door (udp feed) yet.

---

## what each piece does (plain english)

### project skeleton
folders for source code, headers, tests, benchmarks, and fpga work later. keeps the repo organized as it grows.

### cmake build
cmake is the tool that compiles the project. one command builds the main program and the test suite. we require **c++20** (a modern c++ version with useful features).

### domain types (`types.hpp`)
the vocabulary the whole system shares:

| name | plain meaning |
|------|----------------|
| **side** | are you buying or selling? |
| **order type** | limit order (wait at a price) or market order (take whatever is available now) |
| **price** | how much per share, stored as **ticks** (integers), not dollars with decimals |
| **qty** | how many shares |
| **order id** | unique label for each order so we can cancel or modify it later |

**why integers for price?** computers mess up decimal math. exchanges store $100.25 as `10025` ticks with tick size `0.01`. faster and exact.

**strong typedefs** mean the compiler won't let you accidentally pass a quantity where a price is expected. catches bugs early.

### wire protocol (`messages.hpp`)
when two parts of the system talk (or when we replay historical data), they send **fixed-size binary blobs** — not json, not text. binary is smaller and faster to parse.

every message starts with a **header** (version, type, length). then the body depends on type:

| message | what it means |
|---------|----------------|
| **new order** | "place this order" |
| **cancel** | "remove order X" |
| **modify** | "change price/qty on order X" |
| **heartbeat** | "i'm still alive" (for later) |

sizes are locked down (e.g. new order = 31 bytes). if the compiler adds hidden padding, the build **fails** — that protects us from silent protocol breaks.

### tests (catch2)
six small tests that check:

- struct sizes match what we expect
- factory functions fill headers correctly
- the first bytes on the wire look right

run with `ctest` after building. if someone changes a struct and breaks the layout, tests fail immediately.

---

## what we can run today

```bash
./latency_exchange
# prints version, sample price/qty, and wire message size

ctest --output-on-failure
# runs all 6 unit tests
```

that is it. no trading, no order book, no network yet.

---

## technical definitions

### c++20
iso c++ standard published in 2020. we use it for `operator<=>` (spaceship — auto-generates comparisons), and we'll use `std::span`, concepts, etc. later.

### cmake
cross-platform build generator. produces makefiles or ninja files. `FetchContent` downloads catch2 at configure time.

### interface library (`exchange_core`)
a header-only cmake target. no `.cpp` file — just propagates the `include/` path to anything that links it (main binary, tests, future subsystems).

### enum class
scoped enumeration. `Side::Buy` cannot silently convert to `int`, reducing bugs.

### strong typedef / newtype
a thin wrapper `StrongTypedef<Tag, T>` around a primitive. `Price` and `Qty` share the same underlying type (`int64_t`) but are incompatible at compile time. zero runtime overhead.

### trivially copyable
a type with no virtual functions, no user-defined copy logic — safe to `memcpy` into shared memory, dma buffers, or fpga ring buffers.

### wire protocol / on-the-wire format
the exact byte layout of messages as they appear on a network socket or in a replay file. defined with `#pragma pack(1)` for no inter-field padding.

### `#pragma pack(1)`
compiler directive: pack struct members on 1-byte boundaries. ensures deterministic layout across builds for protocol compat.

### little-endian
least significant byte first. x86 and most linux hosts use this natively — no byte swap on ingest for local dev.

### message header
```text
[ version: u8 | type: u8 | length: u16 ]  = 4 bytes
```
parser reads header first, uses `type` to know how many more bytes to expect, validates `length`.

### static_assert
compile-time check. if `sizeof(NewOrderMsg) != 31`, the project does not build.

### catch2
c++ unit test framework. `TEST_CASE` macros, `REQUIRE` assertions. integrated with ctest via `catch_discover_tests`.

### ctest
cmake's test runner. `ctest --output-on-failure` runs all registered tests and prints failures.

### latency / low-latency (goal, not implemented yet)
time from market event (packet arrival) to action (order on book, trade). measured in microseconds or nanoseconds. day 1 does not measure this yet.

### fpga (deferred)
field-programmable gate array. reconfigurable hardware. plan: parse udp packets in verilog/systemverilog, push normalized messages to host memory. software path must work first.

---

## day 1 commits (reference)

| commit message | what landed |
|----------------|-------------|
| `chore: repo skeleton and project layout` | folders, gitignore, readme |
| `build: add cmake c++20 project and minimal executable` | cmake, warnings, main |
| `feat(core): add domain types with strong typedefs for price/qty/order id` | `types.hpp`, `exchange_core` |
| `feat(protocol): define packed v0 binary order/cancel/modify messages` | `messages.hpp` |
| `test: add catch2 and protocol layout unit tests` | 6 passing tests |

---

## what's next (day 2 preview)

- validate raw bytes before casting to structs
- timestamps for latency measurement
- in-memory `order` representation
- order book interface (price-time priority)
- first order book tests

---

## repo layout (current)

```text
latency-exchange/
├── include/exchange/
│   ├── types.hpp              # domain types
│   └── protocol/messages.hpp  # wire format v0
├── src/main.cpp               # smoke-test binary
├── tests/test_messages.cpp    # unit tests
├── cmake/                     # compiler warnings
├── bench/                     # empty — benchmarks later
├── fpga/                      # empty — hardware later
└── docs/                      # documentation (this file)
```
