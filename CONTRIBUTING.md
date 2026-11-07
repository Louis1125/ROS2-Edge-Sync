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

External contributors submit changes via pull requests from a fork.

### Workflow

1. **Fork** the repository and clone your fork.
2. **Create a feature branch** off `master` (never commit directly to `master`).
3. **Make focused changes** — one logical change per PR; keep commits atomic with clear messages.
4. **Run the checks** before submitting: `colcon build && colcon test`, and
   `clang-format` on C++ changes.
5. **Submit the PR** with a clear title and a description covering *what*,
   *why*, and *how it was tested* (use the PR template).
6. **Link the issue** it resolves, if one exists.

### Review requirements

- All GitHub Actions checks must pass (build-test, lint, frontend, docker).
- At least one maintainer approval is required before merge.
- Avoid force-pushing after review has started; push new commits instead.
- Keep the PR small and reviewable; split unrelated changes.

### First contribution

Look for issues labelled `good first issue` (add the label as the project grows).
If you are unsure how to start, open an issue describing what you want to work on.

