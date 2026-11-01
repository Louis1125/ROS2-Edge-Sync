#!/usr/bin/env bash
# Restore PostgreSQL from a dump. Usage: restore.sh <file>
set -euo pipefail
NS=edge-sync
FILE="${1:?usage: restore.sh <file>}"
POD=$(kubectl -n "$NS" get pod -l app=edge-sync-postgres -o jsonpath='{.items[0].metadata.name}')
kubectl -n "$NS" exec -i "$POD" -- psql -U edge_sync edge_sync < "$FILE"
echo "Restored from $FILE"
