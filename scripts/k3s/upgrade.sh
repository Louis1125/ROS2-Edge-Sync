#!/usr/bin/env bash
set -euo pipefail
NS=edge-sync
helm upgrade edge-sync charts/edge-sync -n "$NS" --wait --timeout 5m
echo "Upgraded."
