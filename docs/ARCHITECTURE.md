# Architecture

ROS2-Edge-Sync is a **C++20 modular monolith**: a single deployable program
built from independently compiled, tested and reusable C++ libraries, with ROS 2
as an adapter rather than a core dependency.

## Layers

```
Application Service Layer      sync_coordinator, sync_planner, services
Domain / Core Logic            robot, artifact, manifest, chunk, version
Connection Foundation          peer/session/connection/retry/timeout/flow + ITransport
Base Foundation                types, result/error, config, log, lifecycle, executor, clock
Infrastructure                 Asio TCP/TLS, SQLite, content-addressed filesystem
Application Interfaces         ROS 2 adapter (rclcpp), CLI
```

## Package dependency rules

- `edge_sync_base` depends on no product module.
- `edge_sync_domain` depends on no infrastructure.
- `edge_sync_application` does not depend directly on SQLite or Boost.Asio.
- `edge_sync_transport_asio` only implements the Connection Foundation
  `ITransport` interface.
- ROS 2 is never a required dependency of the domain.
- `edge_sync_runtime` is the only place concrete implementations are assembled.

Do not create a catch-all `common/` or `utils/` package.

## The two foundations

### Base Foundation

Provides capabilities shared by every module: common types (`RobotId`,
`ArtifactId`, `Hash`, `ByteBuffer`), a unified `Result`/`Error` model, logging,
configuration, lifecycle, executor, event bus and clock abstraction. Storage,
network and sync-engine failures all flow through the same error handling.

### Connection Foundation

The single entry point for all external communication. `ConnectionManager`
owns peers, sessions, protocol codec, retry, timeouts, flow control and
security. Upper layers depend on the `ITransport` interface and never open
sockets directly, so TCP, HTTP, gRPC, WebSocket or P2P can be swapped without
touching the sync engine.

```
Sync Coordinator -> Connection Manager -> Session Manager -> Protocol Codec
                    -> ITransport -> Asio TCP/TLS -> Remote Robot
```

## Five principles (plus one)

| Principle                 | Rule                                                        |
| ------------------------- | ----------------------------------------------------------- |
| Unified Foundation        | All modules share the Base Foundation                       |
| Centralized Connectivity  | All cross-robot traffic goes through the Connection Manager |
| Dependency Inversion      | Core does not depend on concrete infrastructure             |
| Native C++                | Core logic is C++20, no Python runtime                      |
| Container Reproducibility | Build, test, simulation all reproducible in Docker          |

**No premature microservices** — packages are libraries inside one program until
a feature genuinely needs independent deployment.

## Composition root

`edge_sync_runtime` builds configuration, logger, database, object store,
connection manager, transport, sync coordinator, and ROS 2 nodes, and manages
the shutdown sequence. Other modules only declare the interfaces they provide.
