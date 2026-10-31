# ADR-003 — Connection Foundation

- **Status:** accepted
- **Date:** 2026-11-01

## Context

Spreading transport/retry/timeout logic across modules leads to duplicated
sockets, inconsistent timeouts and unmanaged connection state.

## Decision

Introduce a Connection Foundation as the single entry point for all
cross-robot communication: peer registry, connection/session management,
protocol codec, retry, timeout and flow control, exposing a transport
interface (`ITransport`) to upper layers.

## Consequences

- Upper layers never open sockets or handle retries directly.
- Transports (TCP, HTTP, gRPC, WebSocket, P2P) are swappable.
- ROS 2 DDS remains separate from artifact transfer.
