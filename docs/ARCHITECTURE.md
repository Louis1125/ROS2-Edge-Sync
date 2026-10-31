# Architecture

ROS2-Edge-Sync is a **C++20 modular monolith**: one deployable program
(`edge_syncd`) composed of independently compiled, tested and reusable C++
libraries, with ROS 2 as an adapter rather than a core dependency.

## Package layout (8 packages)

| Package                    | Build system | Responsibility                                    |
| -------------------------- | ------------ | ------------------------------------------------- |
| `edge_sync_base`           | plain CMake  | Types, `Result`/`Error`, logging, config, lifecycle |
| `edge_sync_protocol`       | plain CMake  | Protocol Buffers wire protocol + envelope codec    |
| `edge_sync_core`           | plain CMake  | Domain model + repository interfaces              |
| `edge_sync_connection`     | plain CMake  | Connection Foundation + Asio TCP transport (2 targets) |
| `edge_sync_storage`        | plain CMake  | SQLite + content-addressed object store            |
| `edge_sync_interfaces`     | ament_cmake  | ROS 2 msg/srv/action definitions                   |
| `edge_sync_ros2`           | ament_cmake  | ROS 2 adapter node                                 |
| `edge_sync_runtime`        | plain CMake  | Composition root (`edge_syncd`)                    |

## Dependency graph

```
edge_sync_base ────────────────┐
                               ├─► edge_sync_protocol ──┐
edge_sync_core ──► edge_sync_base                       ├─► edge_sync_connection ──► edge_sync_connection_asio
edge_sync_storage ─► base, core                         │
edge_sync_interfaces ─► (ROS builtin_interfaces, std_msgs)  ─► edge_sync_ros2
edge_sync_runtime ─► base, protocol, core, connection, connection_asio, storage
```

CMake targets exported: `edge_sync::base`, `edge_sync::protocol`, `edge_sync::core`,
`edge_sync::connection`, `edge_sync::connection_asio`, `edge_sync::storage`.

## Architectural boundaries

- **Core and domain logic do not depend on ROS 2.** `edge_sync_core` links only
  `edge_sync_base`. The sync coordinator (M4) will depend on the *pure*
  `edge_sync::connection` target, never on Asio.
- **Core does not depend on concrete infrastructure.** It declares the
  `core::IArtifactRepository` port; `edge_sync_storage` implements it and the
  runtime injects the concrete implementation.
- **The Connection Foundation is the single path for cross-robot traffic.**
  `edge_sync_connection` exposes the `ITransport` interface and the
  `ConnectionManager`; the Asio transport is a separate internal target
  (`edge_sync_connection_asio`) so upper layers never see Asio.
- **The runtime owns composition.** Only `edge_sync_runtime` wires
  `TcpTransport` into `ConnectionManager`, opens storage, and reads config.
- **No circular dependencies, no global mutable state.** Logging is injected by
  reference; the transport runs a single I/O thread so `ConnectionManager` needs
  no locks.

## Why packages were merged

- `edge_sync_domain` + `edge_sync_application` → **`edge_sync_core`**: the
  application layer is a thin orchestration over the domain and shares its
  boundary; a separate package created artificial seams with no independent
  consumers.
- `edge_sync_transport_asio` → **`edge_sync_connection`** (internal target): the
  transport is an implementation of the Connection Foundation's `ITransport`,
  not a sibling module. Keeping it as a separate target preserves the boundary
  without a separate package.
- `edge_sync_cli` / `edge_sync_test_support` removed: no CLI requirement yet and
  test doubles belong under `tests/support/`, not as production packages.

## Vertical slice

The current slice: start runtime → load config → init logging + storage → start
Asio TCP server → connect a peer → protocol handshake → session lifecycle →
reconnect on failure → graceful shutdown. See `examples/two_robot_sync/`.
