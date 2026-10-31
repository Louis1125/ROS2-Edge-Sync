# Protocol

Remote robot-to-robot messages use **Protocol Buffers** under the
`edge_sync.protocol.v1` package, defined in
`src/foundation/edge_sync_protocol/proto/v1/`.

## Envelope

`MessageEnvelope` wraps every remote message with robot id, request id and
protocol version, and a oneof payload (sync, manifest, chunk, result).

## Files

- `common.proto` — identity, endpoint, hash, chunk primitives.
- `sync.proto` — `MessageEnvelope`, `SyncRequest`, `SyncResult`.
- `manifest.proto` — `ArtifactManifest`, `FileEntry`, `ManifestRequest`.
- `transfer.proto` — `ChunkRequest`, `ChunkResponse`.

## Streaming

Protobuf serializes messages; it does **not** mean whole artifacts are loaded
into memory. Large artifacts are transferred as chunk streams (see M4).

## Separation from ROS 2

`.proto` files define the cross-robot wire protocol and are independent of DDS.
ROS 2 `.msg`/`.srv`/`.action` files (`edge_sync_interfaces`) define the local
node interface. The two are kept separate by design.
