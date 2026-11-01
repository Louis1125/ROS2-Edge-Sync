# ADR-008 — Core peer port (dependency inversion)

- **Status:** accepted
- **Date:** 2026-11-01

## Context

Core application logic must issue remote requests, but it must not depend on
Boost.Asio, protobuf, or the transport.

## Decision

Define `core::IRemotePeer` and `core::IArtifactRepository` as ports in
`edge_sync_core`. `edge_sync_connection` adapts `ConnectionManager` to
`core::IRemotePeer` (`SyncPeerAdapter`), translating between core domain types
and the protobuf wire protocol. `edge_sync_storage` implements
`core::IArtifactRepository`. The runtime injects the concrete implementations.

This inverts the earlier "core depends on connection" idea: now `connection`
depends on `core` (the port), keeping core transport-agnostic.

## Consequences

- `core::SyncCoordinator` is the single application entry point that both the
  ROS 2 adapter and the future HTTP backend will call.
- Request/response correlation lives in `ConnectionManager::request()` via a
  `request_id`, the primitive the sync engine builds on.
