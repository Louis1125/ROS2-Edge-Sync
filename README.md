# ROS2-Edge-Sync

**Reliable distributed state and artifact synchronization for ROS 2 fleets.**

A modular C++20 infrastructure that synchronizes configuration, maps, task
descriptions and software versions across multiple ROS 2 robots, even over
unstable networks.

> **Status:** M0 — repository & build foundation. The package skeleton, Docker
> environment, CMake/colcon build and CI are in place. Synchronization logic
> lands in M1+.

## Design principles

- Offline-first
- Content-addressable storage
- Incremental synchronization
- Fault-tolerant transfer
- Versioned state management
- Secure synchronization
- Modular architecture

## Architecture at a glance

ROS2-Edge-Sync is a **C++20 modular monolith** organized around two shared
foundations, with ROS 2 kept as an adapter rather than a hard dependency of the
core.

```
        Application Service Layer      Sync Coordinator / Planner / Services
                    |
        Domain / Core Logic            Robots, Artifacts, Manifests, Versions
                    |
      ┌─────────────┴─────────────┐
      |      Unified Foundation    |
      |  Connection Foundation     |  Peer / Session / Connection / Retry /
      |                            |  Timeout / Flow / Transport interface
      |  Base Foundation           |  Types / Result / Error / Config / Log /
      |                            |  Lifecycle / Executor / Event Bus / Clock
      └─────────────┬─────────────┘
        Infrastructure               Asio TCP/TLS transport, SQLite + CAS storage
                    |
        Application Interfaces       ROS 2 Adapter (rclcpp), CLI
```

Every cross-robot connection flows through the **Connection Foundation**; every
module shares the **Base Foundation**. The `edge_sync_runtime` package is the
single composition root that wires concrete implementations together.

## Package layout

| Package                    | Responsibility                          | ROS 2 dep |
| -------------------------- | --------------------------------------- | --------- |
| `edge_sync_base`           | Shared types, Result/Error, config, log | no        |
| `edge_sync_protocol`       | Protocol Buffers wire protocol           | no        |
| `edge_sync_connection`     | Unified connection & session management  | no        |
| `edge_sync_domain`         | Domain data and rules                    | no        |
| `edge_sync_application`    | Sync orchestration & services            | no        |
| `edge_sync_storage`        | SQLite + content-addressed storage       | no        |
| `edge_sync_transport_asio` | Asio TCP/TLS transport                   | no        |
| `edge_sync_interfaces`     | ROS 2 msg/srv/action definitions         | yes       |
| `edge_sync_ros2`           | ROS 2 integration (rclcpp)               | yes       |
| `edge_sync_runtime`        | Composition root (`edge_syncd`)          | yes       |
| `edge_sync_cli`            | Native CLI                               | optional  |

## Quick start (Docker)

```bash
# Development shell (build + test inside the container)
docker compose run --rm dev bash
#   colcon build
#   colcon test

# Run a two-robot fleet locally
docker compose up --build robot-a robot-b coordinator
```

## Build from source

Requires ROS 2 Jazzy (Ubuntu 24.04), CMake ≥ 3.20, and a C++20 compiler.

```bash
source /opt/ros/jazzy/setup.bash
rosdep install --from-paths src --ignore-src -r -y
colcon build --symlink-install
```

## Roadmap

- **M0** Repository & build foundation *(this repo)*
- **M1** Base Foundation
- **M2** Connection Foundation
- **M3** Protocol & storage
- **M4** Synchronization engine
- **M5** ROS 2 integration
- **M6** Reliability & security
- **M7** Distributed deployment

See [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) and
[docs/decisions/](docs/decisions/) for the design rationale.

## License

MIT — see [LICENSE](LICENSE).
