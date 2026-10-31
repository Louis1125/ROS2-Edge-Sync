# ADR-002 — C++20

- **Status:** accepted
- **Date:** 2026-11-01

## Context

A Python-first design was proposed earlier. Performance and ownership of the
network/data path matter for an infrastructure product.

## Decision

Use C++20 as the implementation language for the entire core: data model,
synchronization engine, networking, storage and ROS 2 integration. C++23 is
deferred to preserve compiler/dependency compatibility.

## Consequences

- Concepts, coroutines, `std::span`, `std::jthread`, `std::stop_token` and
  `std::chrono` are available for async I/O and lifecycle management.
- Performance is still proven with benchmarks, not assumed from the language.
