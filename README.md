# ROS2-Edge-Sync

[![build-test](https://github.com/Louis1125/ROS2-Edge-Sync/actions/workflows/build.yml/badge.svg)](https://github.com/Louis1125/ROS2-Edge-Sync/actions)

Reliable distributed artifact synchronization and fleet coordination for ROS 2
robot fleets over unreliable networks.

**Status: Alpha.** The core synchronization engine, control plane, and K3s
deployment are implemented and tested, but the project is **not
production-ready** — see "Status & limitations" below.

## What it does

- **Receiver-driven synchronization** — the control plane dispatches a command
  to the *target* robot, which pulls the artifact from the *source* robot over
  the peer-to-peer data plane, verifies SHA-256, and activates it atomically.
- **Durable command dispatch** — a PostgreSQL transactional outbox, delivery vs.
  execution state, and agent reconnection/offline recovery.
- **Control plane** — Drogon + PostgreSQL, robot registry with heartbeat-based
  online/offline, sync-job API with bearer auth.
- **Deployment** — Docker (local), Helm/K3s (edge), unified `./edge-sync` CLI.

## Quick start

```bash
git clone <repo-url> && cd ros2-edge-sync
./edge-sync up --mode local     # builds/pulls images, migrates, waits for health
./edge-sync status              # service states, URLs, robots, jobs
./edge-sync doctor              # diagnose the environment
./edge-sync down                # stop (preserves data; --reset deletes it)
```

Web console at http://localhost:5173 (API at :8080). See
[COMMANDS.md](COMMANDS.md) and [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

## Status & limitations (accurate as of 2026-11-07)

- ✅ K3s deployment **and** functional acceptance pass on a real k3d cluster:
  two-agent sync (SHA-256 byte-identical), restart recovery, network
  interruption, Helm upgrade/rollback, PostgreSQL backup/restore — see
  [docs/release/K3S_ACCEPTANCE.md](docs/release/K3S_ACCEPTANCE.md).
- ⚠️ **Receiver-driven architecture**: the source robot is a peer-to-peer
  source, not a registered control-plane agent.
- ⚠️ Verified with a **200 KB** artifact; artifacts **> 100 MB** are untested.
- ⚠️ **Repeated-sync deduplication** under repetition is untested.
- ⚠️ **Gateway TLS is not implemented** — the robot gateway is plaintext (HMAC
  handshake only). Do not expose it to untrusted networks.
- ⚠️ This is **Alpha software**, not production-ready.

## License

MIT — see [LICENSE](LICENSE) and [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
