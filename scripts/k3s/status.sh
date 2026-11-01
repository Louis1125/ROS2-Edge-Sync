#!/usr/bin/env bash
set -euo pipefail
NS=edge-sync
kubectl -n "$NS" get pods,svc,ingress,pvc
