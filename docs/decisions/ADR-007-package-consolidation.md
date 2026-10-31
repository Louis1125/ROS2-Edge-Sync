# ADR-007 — Package consolidation

- **Status:** accepted
- **Date:** 2026-11-01

## Context

The M0 scaffold had 12 packages with duplicated seams (domain/application,
connection/transport), unused packages (CLI, test support), and everything built
as `ament_cmake` even where no ROS dependency existed.

## Decision

Consolidate to 8 packages. Merge domain+application into `edge_sync_core`,
merge connection management + Asio transport into `edge_sync_connection` (as two
internal CMake targets), remove `edge_sync_cli` and `edge_sync_test_support`
(test utilities move to `tests/support/`). Use plain CMake for ROS-independent
libraries and `ament_cmake` only for the ROS interface/adapter packages.

## Consequences

- Clearer ownership and dependency boundaries (see ARCHITECTURE.md).
- ROS-independent packages build and test without a ROS environment.
- The pure `edge_sync::connection` target keeps Asio out of core.
