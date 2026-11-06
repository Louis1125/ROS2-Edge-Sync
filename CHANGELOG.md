# Changelog

## Unreleased

- Unified `./edge-sync` CLI (up/down/status/doctor) with staged progress and URL discovery.
- Robot heartbeat with online/offline state.
- Bearer-token API authentication.
- Durable command dispatch (transactional outbox, delivery vs. execution state).
- Functional frontend (overview, robots, sync-jobs, deployment wizard).
- Helm chart + K3s scripts + local/edge/production presets.
- Successful two-robot E2E with byte-identical transfer verification.
