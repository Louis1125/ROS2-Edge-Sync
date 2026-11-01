#!/usr/bin/env bash
# Dump PostgreSQL to a local file. Usage: backup.sh [outfile]
set -euo pipefail
NS=edge-sync
OUT="${1:-edge-sync-backup-$(date +%Y%m%d%H%M%S).sql}"
POD=$(kubectl -n "$NS" get pod -l app=edge-sync-postgres -o jsonpath='{.items[0].metadata.name}')
kubectl -n "$NS" exec "$POD" -- pg_dump -U edge_sync edge_sync > "$OUT"
echo "Backup written to $OUT"
