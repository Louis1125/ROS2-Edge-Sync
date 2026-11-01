#!/usr/bin/env bash
# Install ROS2-Edge-Sync on K3s via Helm. Usage: install.sh [preset]
set -euo pipefail
PRESET="${1:-local}"
NS=edge-sync
RELEASE=edge-sync

for bin in kubectl helm; do
  command -v "$bin" >/dev/null || { echo "error: $bin not found (install it first)"; exit 1; }
done
kubectl cluster-info >/dev/null 2>&1 || { echo "error: cannot reach the Kubernetes cluster"; exit 1; }

kubectl create namespace "$NS" --dry-run=client -o yaml | kubectl apply -f - >/dev/null

helm upgrade --install "$RELEASE" charts/edge-sync -n "$NS" \
  -f "charts/edge-sync/values-$PRESET.yaml" \
  --set fleetSecret="${EDGE_SYNC_FLEET_SECRET:-$(openssl rand -hex 16)}" \
  --set postgres.password="${POSTGRES_PASSWORD:-edge_sync}" \
  --wait --timeout 5m

echo "Installed ($PRESET preset)."
kubectl -n "$NS" get pods,svc
echo "Control plane: http://<node>:8080  (or the configured ingress host)"
