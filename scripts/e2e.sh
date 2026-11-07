#!/usr/bin/env bash
# Full two-robot E2E: control plane + postgres + robot-a (source) + robot-b
# (agent). Proves: API -> outbox -> gateway -> robot-b -> SyncEngine -> robot-a
# -> transfer -> verify -> activate -> persisted success.
set -euo pipefail

WORKSPACE=/home/louis/edge-sync-build
NET=edge-e2e
SECRET=test-secret
rm -rf /tmp/e2e-a /tmp/e2e-b && mkdir -p /tmp/e2e-a /tmp/e2e-b

head -c 153600 /dev/urandom > /tmp/e2e-a/artifact.bin

docker rm -f e2e-pg e2e-cp e2e-a e2e-b 2>/dev/null || true
docker network rm "$NET" 2>/dev/null || true
docker network create "$NET" >/dev/null

docker run -d --name e2e-pg --network "$NET" -e POSTGRES_DB=edge_sync -e POSTGRES_USER=edge_sync -e POSTGRES_PASSWORD=edge_sync postgres:16-alpine >/dev/null
for i in $(seq 1 30); do docker exec e2e-pg pg_isready -U edge_sync >/dev/null 2>&1 && break; sleep 2; done

docker run -d --name e2e-cp --network "$NET" -p 8080:8080 --entrypoint /bin/bash \
  -v "$WORKSPACE":/workspace edge-sync-cp-runtime -c '
    export LD_LIBRARY_PATH=/opt/drogon/lib:/workspace/install/edge_sync_base/lib:/workspace/install/edge_sync_core/lib:/workspace/install/edge_sync_protocol/lib:/workspace/install/edge_sync_connection/lib:/workspace/install/edge_sync_connection_asio/lib:$LD_LIBRARY_PATH
    export MIGRATIONS_DIR=/workspace/apps/edge_sync_control_plane/migrations
    export DATABASE_URL="host=e2e-pg port=5432 dbname=edge_sync user=edge_sync password=edge_sync"
    export EDGE_SYNC_FLEET_SECRET='"$SECRET"'
    exec /workspace/build/control/edge_sync_control_plane' >/dev/null

for i in $(seq 1 30); do curl -fsS http://localhost:8080/api/v1/health >/dev/null 2>&1 && break; sleep 1; done
curl -fsS http://localhost:8080/api/v1/health >/dev/null 2>&1 || { echo "control plane not healthy"; docker logs e2e-cp | tail -20; exit 1; }

cat > /tmp/e2e-a.yaml <<A_EOF
robot:
  id: robot-a
  role: robot
connection:
  listen: 0.0.0.0
  port: 7443
  peers: ""
fleet:
  secret: $SECRET
storage:
  root: /data
  database: meta.sqlite
logging:
  level: info
A_EOF

docker run --rm --user 0 --entrypoint /bin/bash -v "$WORKSPACE":/workspace -v /tmp/e2e-a:/data -v /tmp/e2e-a.yaml:/config.yaml:ro \
  engineai_robotics_env:humble-desktop-local -c '
    export LD_LIBRARY_PATH=$(find /workspace/install -maxdepth 2 -type d -name lib | tr "\n" ":")$LD_LIBRARY_PATH
    /workspace/install/edge_sync_runtime/bin/edge_syncd --config /config.yaml --seed /data/artifact.bin --artifact map-1' 2>&1 | tail -1

docker run -d --name e2e-a --network "$NET" --user 0 --entrypoint /bin/bash \
  -v "$WORKSPACE":/workspace -v /tmp/e2e-a:/data -v /tmp/e2e-a.yaml:/config.yaml:ro \
  engineai_robotics_env:humble-desktop-local -c '
    export LD_LIBRARY_PATH=$(find /workspace/install -maxdepth 2 -type d -name lib | tr "\n" ":")$LD_LIBRARY_PATH
    exec /workspace/install/edge_sync_runtime/bin/edge_syncd --config /config.yaml' >/dev/null
sleep 3
A_IP=$(docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' e2e-a)
CP_IP=$(docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' e2e-cp)

cat > /tmp/e2e-b.yaml <<B_EOF
robot:
  id: robot-b
  role: robot
connection:
  listen: 0.0.0.0
  port: 7444
  peers: $A_IP:7443
control_plane:
  host: $CP_IP
  port: 7443
fleet:
  secret: $SECRET
storage:
  root: /data
  database: meta.sqlite
logging:
  level: info
B_EOF

docker run -d --name e2e-b --network "$NET" --user 0 --entrypoint /bin/bash \
  -v "$WORKSPACE":/workspace -v /tmp/e2e-b:/data -v /tmp/e2e-b.yaml:/config.yaml:ro \
  engineai_robotics_env:humble-desktop-local -c '
    export LD_LIBRARY_PATH=$(find /workspace/install -maxdepth 2 -type d -name lib | tr "\n" ":")$LD_LIBRARY_PATH
    exec /workspace/install/edge_sync_runtime/bin/edge_syncd --config /config.yaml' >/dev/null
sleep 5

echo "=== create job (robot-a -> robot-b, map-1) ==="
curl -s -X POST -H 'Content-Type: application/json' \
  -d '{"source_robot":"robot-a","target_robot":"robot-b","artifact_id":"map-1"}' \
  http://localhost:8080/api/v1/sync-jobs; echo

for i in $(seq 1 40); do
  JOB_STATUS=$(curl -s http://localhost:8080/api/v1/sync-jobs | python3 -c "import sys,json; d=json.load(sys.stdin); print(d[0]['status'] if d else '')" 2>/dev/null || true)
  case "$JOB_STATUS" in succeeded|failed|cancelled) break;; esac
  sleep 1
done
echo "=== job state ==="
curl -s http://localhost:8080/api/v1/sync-jobs; echo
echo "=== control plane dispatch/status ==="
docker logs e2e-cp 2>&1 | grep -iE 'dispatch|status|agent|connected' | tail -8
echo "=== robot-b active artifact ==="
ls -la /tmp/e2e-b/artifacts/active/ 2>&1
echo "=== integrity check (should be identical) ==="
sha256sum /tmp/e2e-a/artifact.bin /tmp/e2e-b/artifacts/active/map-1 2>&1

docker rm -f e2e-pg e2e-cp e2e-a e2e-b >/dev/null 2>&1 || true
docker network rm "$NET" >/dev/null 2>&1 || true
echo "E2E done"
