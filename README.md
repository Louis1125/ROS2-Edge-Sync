# ROS2-Edge-Sync

**Reliable distributed state and artifact synchronization for ROS 2 fleets.**

A C++20 modular monolith that synchronizes configuration, maps, task
descriptions and software versions across ROS 2 robots over unreliable networks.

> **Status:** working vertical slice. The runtime starts, loads config, opens
> storage, runs an Asio TCP server, connects to a peer, completes the protocol
> handshake, manages sessions/reconnect and shuts down gracefully — verified
> across two processes and two Docker containers. Synchronization algorithms are
> next (M4).

## Design principles

Offline-first · content-addressable storage · incremental sync · fault-tolerant
transfer · versioned state · secure sync · modular architecture.

## Packages

| Package | Responsibility |
| --- | --- |
| `edge_sync_base` | types, Result/Error, logging, config |
| `edge_sync_protocol` | protobuf wire protocol + envelope codec |
| `edge_sync_core` | domain model + repository interfaces |
| `edge_sync_connection` | Connection Foundation + Asio TCP transport |
| `edge_sync_storage` | SQLite + content-addressed object store |
| `edge_sync_interfaces` | ROS 2 msg/srv/action |
| `edge_sync_ros2` | ROS 2 adapter node |
| `edge_sync_runtime` | composition root (`edge_syncd`) |

## Build

```bash
source /opt/ros/$ROS_DISTRO/setup.bash
colcon build && colcon test
```

See [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md), [docs/BUILD.md](docs/BUILD.md),
and [docs/decisions/](docs/decisions/).

## License

MIT.
