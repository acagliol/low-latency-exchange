# latency-exchange

C++20 electronic exchange skeleton with optional FPGA acceleration (later).

## build (linux / wsl2)

    mkdir build && cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    cmake --build .
    ctest --output-on-failure

## docs

see [docs/what-we-built.md](docs/what-we-built.md) for a plain-english walkthrough of day 1.

## status

- [x] day 1: skeleton, types, wire protocol, tests (see docs)
- [ ] market data feed handler
- [ ] limit order book
- [ ] matching engine
