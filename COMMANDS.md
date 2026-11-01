# COMMANDS.md

Four commands run ROS2-Edge-Sync. Everything else is advanced usage.

## Prerequisites

Docker (and, for `--mode edge|production`, K3s + kubectl + Helm).

## Quick install

```bash
git clone <repo-url> && cd ros2-edge-sync
./edge-sync up --mode local          # builds/pulls images, migrates, waits for health
```

Open the web console at **http://localhost:5173** (API at **:8080**).

## The four commands

```bash
./edge-sync up       # start the platform (local Docker by default)
./edge-sync status   # mode, services, database, robots, sync jobs
./edge-sync doctor   # diagnose the environment with corrective suggestions
./edge-sync down     # stop cleanly (preserves data; --reset deletes it)
```

Deployment modes: `./edge-sync up --mode edge` and `--mode production` install
via Helm onto K3s/Kubernetes.

`up` automatically: checks prerequisites, builds/pulls images, starts
PostgreSQL + control plane + gateway + frontend, runs migrations, waits for
health, and prints the URLs. `down` preserves PostgreSQL data unless `--reset`.
`status` shows real state (health, connected robots, active jobs). `doctor`
detects missing deps and misconfiguration with suggested fixes.

---

# Advanced

Build/test C++ (ROS 2 Humble container):

```bash
docker run --rm -v "$PWD":/workspace -w /workspace \
  engineai_robotics_env:humble-desktop-local bash -c \
  'source /opt/ros/humble/setup.bash && colcon build && colcon test'
```

Robot daemon (edge device; connects outbound to the control plane):

```bash
edge_syncd --config config/robot.yaml
edge_syncd --config config/robot.yaml --seed /data/map.bin --artifact map-1
```

API: `openapi.yaml`; quick checks:

```bash
curl -s http://localhost:8080/api/v1/health
curl -s -X POST -H 'Content-Type: application/json' \
  -d '{"source_robot":"robot-a","target_robot":"robot-b","artifact_id":"map-1"}' \
  http://localhost:8080/api/v1/sync-jobs
```

Helm/K3s internals live under `scripts/k3s/` and `charts/edge-sync/`. The
two-robot E2E is `scripts/e2e.sh`.
