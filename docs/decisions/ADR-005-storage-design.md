# ADR-005 — Storage design

- **Status:** accepted
- **Date:** 2026-11-01

## Context

Artifacts need deduplicated, integrity-checked storage with atomic activation.

## Decision

Use SQLite for metadata plus a content-addressed filesystem object store
(keyed by SHA-256, optionally BLAKE3), with Zstandard compression. New versions
are staged and activated atomically.

## Consequences

- Incremental sync via content addressing.
- Integrity verified before activation.
- SQLite/CAS are infrastructure details hidden behind domain interfaces.
