#!/usr/bin/env bash
# Diagnose an installation. Prints OK / FAIL with corrective hints.
set -uo pipefail
NS=edge-sync
fail=0
chk() { if "$@"; then echo "OK   $*"; else echo "FAIL $*"; fail=1; fi; }

kubectl cluster-info >/dev/null 2>&1 || { echo "FAIL cluster unreachable"; exit 1; }

chk kubectl get namespace "$NS" >/dev/null 2>&1
chk kubectl -n "$NS" get pods >/dev/null 2>&1
READY=$(kubectl -n "$NS" get pods -o jsonpath='{.items[?(@.status.phase=="Running")].metadata.name}' 2>/dev/null)
[ -n "$READY" ] && echo "OK   running pods present" || { echo "FAIL no running pods (check: kubectl -n $NS describe pod)"; fail=1; }

kubectl -n "$NS" get svc edge-sync-control-plane >/dev/null 2>&1 \
  && echo "OK   control-plane service exists" || { echo "FAIL control-plane service missing"; fail=1; }

if kubectl -n "$NS" get ingress edge-sync >/dev/null 2>&1; then
  echo "OK   ingress present"
else
  echo "INFO ingress not configured (set ingress.enabled=true to expose)"
fi

if kubectl -n "$NS" get pod -l app=edge-sync-control-plane -o name >/dev/null 2>&1; then
  CP_POD=$(kubectl -n "$NS" get pod -l app=edge-sync-control-plane -o jsonpath='{.items[0].metadata.name}')
  if kubectl -n "$NS" exec "$CP_POD" -- sh -c 'command -v curl && curl -fsS http://localhost:8080/api/v1/health' >/dev/null 2>&1; then
    echo "OK   control-plane health endpoint"
  else
    echo "FAIL control-plane health endpoint unreachable (check: kubectl -n $NS logs $CP_POD)"
    fail=1
  fi
fi

exit "$fail"
