# COMMANDS.md

Copy-paste commands for ROS2-Edge-Sync. Every command below exists in the
repository; none are illustrative placeholders.

## A. Quick Start

Prerequisites: Docker, Docker Compose. (A C++20 toolchain + ROS 2 Humble are only
needed to build from source — see section B.)

```bash
git clone <repo-url> && cd ros2-edge-sync

# 1. Build the data-plane and control-plane images (first time)
docker build -f docker/drogon-base.Dockerfile -t edge-sync-drogon-base .
docker build -f docker/control-plane.Dockerfile -t edge-sync-cp-runtime .

# 2. Build the C++ data plane inside the ROS container
docker run --rm -v "$PWD":/workspace -w /workspace \
  engineai_robotics_env:humble-desktop-local bash -c \
  'source /opt/ros/humble/setup.bash && colcon build'

# 3. Run the full two-robot demo (starts PostgreSQL, control plane, two robots)
bash scripts/e2e.sh
```

Expected: `scripts/e2e.sh` ends with `E2E done` and prints
`status: succeeded` for the job plus two identical sha256 hashes.

## B. Development Commands

Run from the repository root inside the ROS container
(`engineai_robotics_env:humble-desktop-local`, use `--entrypoint /bin/bash`).

```bash
# Build all 8 C++ packages
source /opt/ros/humble/setup.bash && colcon build

# Build one package
colcon build --packages-select edge_sync_connection

# Run all tests (unit + integration; 38 tests)
colcon test && colcon test-result --verbose

# Frontend development server (Node 20, proxies /api to :8080)
docker run --rm -v "$PWD/web":/app -w /app -p 5173:5173 node:20-alpine \
  sh -c 'npm install && npm run dev -- --host'

# Frontend production build
docker run --rm -v "$PWD/web":/app -w /app node:20-alpine \
  sh -c 'npm install && npm run build'
```

The control plane (`apps/edge_sync_control_plane`) is not part of `colcon
build`; build it separately (see section C) because it needs Drogon + libpq.

## C. Docker Commands

```bash
# Build images
docker build -f docker/drogon-base.Dockerfile -t edge-sync-drogon-base .   # Drogon (pinned)
docker build -f docker/control-plane.Dockerfile -t edge-sync-cp-runtime .  # + protobuf/boost deps
docker build -f docker/frontend.Dockerfile -t edge-sync-web .

# Full demo (compose services: postgres, control-plane, frontend, robot-a, robot-b)
docker compose up -d

# View logs / status / restart / reset
docker compose logs -f control-plane
docker compose ps
docker compose restart control-plane
docker compose down -v   # also removes volumes (reset)

# Multi-robot E2E (repeatable, asserts byte-identical sync)
bash scripts/e2e.sh
```

## D. Robot Commands

The robot daemon is `edge_syncd` (in `install/edge_sync_runtime/bin`).

```bash
# Start a robot agent (config: robot id, peers, control-plane endpoint, secret)
edge_syncd --config config/robot.yaml

# Publish an artifact (seed) then exit
edge_syncd --config config/robot.yaml --seed /data/artifact.bin --artifact map-1
```

Config keys (indented YAML): `robot.id`, `connection.listen/port/peers`,
`control_plane.host/port`, `fleet.secret`, `storage.root/database`,
`logging.level`. `fleet.secret` is the shared secret for the authenticated
handshake; `control_plane.host` makes the robot connect outbound to the control
plane as an agent.

## E. API Commands

The control plane serves HTTP on `:8080` (gateway on `:7443`).

```bash
curl -s http://localhost:8080/api/v1/health
# {"status":"ok"}

curl -s http://localhost:8080/api/v1/robots

curl -s -X POST -H 'Content-Type: application/json' \
  -d '{"source_robot":"robot-a","target_robot":"robot-b","artifact_id":"map-1"}' \
  http://localhost:8080/api/v1/sync-jobs

curl -s http://localhost:8080/api/v1/sync-jobs
curl -s http://localhost:8080/api/v1/sync-jobs/<job_id>
```

OpenAPI: `openapi.yaml` at the repository root.

## F. K3s / Kubernetes Commands

A Helm chart exists at `charts/edge-sync` (control plane, frontend, optional
PostgreSQL, migration Job, Ingress, NetworkPolicy).

```bash
# Validate the chart (no cluster required)
docker run --rm -v "$PWD/charts":/charts -w /charts alpine/helm lint edge-sync
docker run --rm -v "$PWD/charts":/charts -w /charts alpine/helm template demo edge-sync

# Install (from a machine with kubectl + Helm pointed at K3s)
helm install edge-sync charts/edge-sync --namespace edge-sync --create-namespace \
  --set postgres.enabled=true --set ingress.enabled=true

# Upgrade / rollback / uninstall
helm upgrade edge-sync charts/edge-sync --namespace edge-sync
helm rollback edge-sync --namespace edge-sync
helm uninstall edge-sync --namespace edge-sync
```

> k3s/kind installation scripts (`scripts/k3s/*.sh`) and real cluster
> validation are the next planned addition; the chart is lint/render-verified
> only at this point.

## G. Troubleshooting

- **PostgreSQL connection failure** (`postgres connect: …`): verify
  `DATABASE_URL` and that the DB is reachable: `docker exec <pg> pg_isready`.
- **Robot cannot connect**: check `control_plane.host/port` in the robot config
  and that the gateway is listening (`agent gateway listening on port 7443`).
- **Authentication failure**: the `fleet.secret` on robot and control plane must
  match; the log shows `authentication failed`.
- **Job stuck `queued`**: the dispatcher only sends to connected robots — check
  `agent connected: <robot>` in the control-plane log.
- **`acknowledged` without `succeeded`**: the robot got the command but the
  SyncEngine failed; inspect the robot log for the sync error.
- **Frontend cannot reach API**: the dev server proxies `/api` to `:8080`; in
  containers use the `control-plane` service name.
