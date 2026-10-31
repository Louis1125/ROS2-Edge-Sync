# Contributing to ROS2-Edge-Sync

## Development environment

Use the provided Docker development container for a reproducible toolchain:

```bash
docker compose build dev
docker compose run --rm dev bash
```

## Conventions

- C++20, formatted with `clang-format` (config at repo root).
- Code is linted with `clang-tidy`.
- Ownership follows RAII; `new`/`delete` are not used directly.
- Errors are reported through `edge_sync::base::Result`/`Error`, not scattered
  exception types.
- No unmanaged detached threads; no global mutable state.

## Commit style

Conventional commits, scoped by package, e.g.:

```
feat(base): add unified error handling
feat(connection): implement peer registry
fix(storage): verify chunk hash before write
test(connection): add connection lifecycle tests
```

Each commit should be independently buildable and testable.

## Pull requests

1. Open an issue describing the change first where non-trivial.
2. Keep changes focused on a single module.
3. Add tests for new behavior.
4. Ensure `clang-format` and CI pass.
