# ADR-010 — Staging and atomic activation

- **Status:** accepted
- **Date:** 2026-11-01

## Context

An artifact must never become active if it is incomplete or corrupted, and a
torn transfer must never replace the currently active version.

## Decision

Artifacts are assembled in a staging file, with per-chunk hash verification and
a full-artifact hash check (streamed from disk, not loaded into memory). Only
after the full hash matches is the staged file atomically renamed into the
active location and the active version recorded in SQLite. Chunks are stored
immutably in the content-addressed object store, so re-transfers dedupe.

## Consequences

- Incomplete or corrupted artifacts are rejected before activation.
- Active version is tracked and queryable (`IArtifactStore::active_version`).
- Crash recovery is safe: an interrupted transfer leaves only staging/immutable
  chunks, never a half-written active artifact.
