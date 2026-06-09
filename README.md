# latency-exchange

C++20 electronic exchange skeleton with optional FPGA acceleration (later).

## build (linux / wsl2)

    mkdir build && cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    cmake --build .
    ctest --output-on-failure

## status

- [x] skeleton, types, wire protocol, tests
- [ ] market data feed handler
- [ ] limit order book
- [ ] matching engine
