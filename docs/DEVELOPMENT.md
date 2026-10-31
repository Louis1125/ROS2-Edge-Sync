# Development

## Toolchain

- Formatting: `clang-format` (config at repo root).
- Linting: `clang-tidy`.
- Tests: GoogleTest via `ament_cmake_gtest`.
- Benchmarks: Google Benchmark (added when M3+ benchmarking begins).

## Coding standards

- C++20: concepts, coroutines, `std::span`, `std::jthread`, `std::chrono`,
  `std::stop_token`. C++23 is deliberately avoided for now.
- RAII for all resources; `std::unique_ptr` for dynamic ownership;
  `std::shared_ptr` only when shared ownership is genuinely required.
- No raw `new`/`delete`; no unmanaged detached threads.
- Exceptions are converted to `edge_sync::base::Result` at module boundaries.

## Quality pipeline

```
clang-format -> clang-tidy -> CMake build -> GoogleTest
  -> ASan/UBSan -> TSan -> Google Benchmark
```

ASan/UBSan run as part of CI tests; TSan is reserved for the concurrency-heavy
connection/session/transfer code.

## Performance

Do not build memory pools, zero-copy buffers or lock-free queues up front.
Establish benchmarks first (connection latency, hashing throughput, chunk
transfer throughput, P95/P99 sync latency) and optimize against data.

## Commit strategy

Conventional commits scoped by package (see CONTRIBUTING.md). Milestone cadence:

- M0 chore/ci scaffolding
- M1 `feat(base)/test(base)`
- M2 `feat(connection)/feat(transport)/test(connection)`
- M3 `feat(protocol)/feat(storage)`
- M4 `feat(application)`
- M5 `feat(ros2)`
- M6 `feat(security)`
- M7 `feat(fleet)`
