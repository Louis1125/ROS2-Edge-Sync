#!/usr/bin/env bash
set -euo pipefail
NS=edge-sync
helm uninstall edge-sync -n "$NS" || true
echo "Uninstalled. Namespace '$NS' retained (delete with: kubectl delete ns $NS)."
