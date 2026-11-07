# K3S Functional Acceptance

- **Date:** 2026-11-07
- **Cluster:** k3d v5.9.0 → K3s v1.35.5 (single node, `local-path` storage)
- **Deployment:** Helm (`charts/edge-sync`) with locally-built `edge-sync-cp:local` / `edge-sync-web:local`

## Acceptance matrix

| Scenario | Result |
|---|---|
| Cluster creation | ✅ Passed |
| Helm install (all services) | ✅ Passed |
| PostgreSQL + 5Gi PVC | ✅ Passed |
| Control plane health (`/api/v1/health`) | ✅ Passed |
| Frontend access (nginx) | ✅ Passed |
| Gateway LoadBalancer (7443) | ✅ Passed |
| Robot B registers online | ✅ Passed |
| Robot A registers | ⚠️ Partial — robot-a is a peer source reached robot-to-robot, not a control-plane agent |
| Artifact sync through K3s | ✅ Passed |
| SHA-256 verification | ✅ Passed (source == destination) |
| Job status reporting | ✅ Passed (`succeeded`) |
| Control plane restart recovery | ✅ Passed (robot auto-reconnects) |
| Job metadata persistence | ✅ Passed (survives restart) |
| Network interruption → offline | ✅ Passed (16s heartbeat window) |
| Robot reconnect → online | ✅ Passed |
| Helm upgrade | ✅ Passed (1 → 2 replicas) |
| Helm rollback | ✅ Passed |
| PostgreSQL backup | ✅ Passed (pg_dump) |
| PostgreSQL restore (isolated) | ✅ Passed (records preserved) |
| Large artifact (>100 MB) | ❌ Not tested |
| Repeated-sync dedup / no-corruption | ❌ Not tested |

## Evidence (key outputs)

- Sync job: `{"jobId":"sync-1794046932171-0","status":"succeeded"}`.
- Integrity: `b1d11d662d2acc7ced3a0f3cdbca86251e711c0ef5810cb669fdb121e8356465` on both
  source and destination (200 KB artifact, 4 chunks).
- Reconnect: robot-b `offline` after kill, `online` after restart.
- Restore: `robot:robot-b:online`, `job:sync-1794046932171-0:succeeded` recovered in a
  fresh PostgreSQL.

## Notes / limitations

1. **Robot A is not a control-plane agent.** The sync is receiver-driven: the
   control plane dispatches a command to the *target* (robot-b), which then
   pulls from the *source* (robot-a) peer-to-peer. Making the source also an
   enrolled agent is a future fleet-management enhancement, not required for
   the transfer itself.
2. **Large-artifact and repeated-sync tests not run.** Transfer correctness for
   200 KB is proven; multi-hundred-MB and dedup-under-repetition remain open.
3. **No TLS.** The gateway is plaintext; a real deployment needs TLS or a VPN.
