# ADR-004 — Transport protocol

- **Status:** accepted
- **Date:** 2026-11-01

## Context

Remote robots need a versioned, language-neutral wire protocol independent of
DDS.

## Decision

Use Protocol Buffers (`edge_sync.protocol.v1`) with a `MessageEnvelope` wrapper
for all remote messages. Large artifacts are chunked and streamed rather than
loaded whole into memory.

## Consequences

- `.proto` (wire) is kept separate from ROS 2 `.msg/.srv/.action` (node
  interface).
- Versioning via a protocol version field.
