# Architecture

ROS2-Edge-Sync is a C++20 full-stack platform: a data plane that synchronizes
artifacts between robots, and a control plane that manages the fleet over HTTP.

## Packages

| Package | Type | Responsibility |
| --- | --- | --- |
| `edge_sync_base` | plain CMake | types, Result/Error, logging, config, SHA-256 |
| `edge_sync_protocol` | plain CMake | protobuf wire protocol + envelope codec |
| `edge_sync_core` | plain CMake | domain model, ports, planner, SyncEngine |
| `edge_sync_connection` | plain CMake | ConnectionManager + Asio transport + transfer |
| `edge_sync_storage` | plain CMake | SQLite + CAS object store + artifact store |
| `edge_sync_interfaces` | ament | ROS 2 msg/srv/action |
| `edge_sync_ros2` | ament | ROS 2 adapter node |
| `edge_sync_runtime` | plain CMake | robot daemon (`edge_syncd`) |
| `edge_sync_control_plane` | plain CMake | Drogon HTTP API + PostgreSQL (apps/) |

## Data plane dependency graph

```
base ← protocol ← connection ← connection_asio
base ← core ← storage
base ← core ← runtime
core defines ports: IRemotePeer, ITransfer, IArtifactRepository, IArtifactStore, IChunkProvider
connection implements IRemotePeer + ITransfer (SyncPeerAdapter)
storage implements IArtifactRepository + IArtifactStore + IChunkProvider
```

## Synchronization flow

```
SyncEngine.synchronize(source, artifact)
  → request_manifest (IRemotePeer)           # control
  → SyncPlanner.plan(local, remote)          # content-hash diff
  → transfer.pull_chunks(needed, receiver)   # receiver-driven streaming
  → store.commit(manifest)                   # verify + atomic activation
```

Chunk hashes, the full artifact hash and manifest identity are distinct. The
receiver is authoritative for resume; identical content is fetched once and
assembled at every matching offset.

## Boundaries

- Core depends only on its own ports, never on Asio/SQLite/protobuf.
- The runtime is the composition root for the robot; the control plane composes
  Drogon + PostgreSQL + the agent gateway.
- The ROS 2 action and the HTTP backend both invoke `core::SyncEngine`.
