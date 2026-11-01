# ADR-009 — Receiver-driven streaming transfer

- **Status:** accepted
- **Date:** 2026-11-01

## Context

Large artifacts must not be moved with repeated blocking request/response calls,
and the Asio I/O thread must never block on disk or hashing.

## Decision

Chunk transfer is receiver-driven and pipelined. The receiver (SyncEngine)
opens a transfer session, then requests chunks with a bounded sliding window
(default 4). The sender responds 1:1 with `ChunkData`; disk reads happen on a
dedicated sender worker thread. The receiver verifies each chunk hash on the
caller thread, acknowledges it, and re-requests any outstanding chunks on
timeout (receiver-authoritative resume). `ChunkAck` is a durability signal; the
receiver's own committed-chunk record is authoritative for resume.

## Consequences

- Bounded in-flight memory on both sides (the window bounds requests and data).
- Control messages (`TransferRequest`/`Response`) and bulk `ChunkData` are
  separate messages.
- Identical chunk content is fetched once and assembled at every matching
  offset (content-addressed dedup).
