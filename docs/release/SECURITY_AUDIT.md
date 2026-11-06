# Security Audit — Git History

- **Date:** 2026-11-06
- **Scanner:** Gitleaks v8.30.1 (static release binary)
- **Scope:** full local git history — 45 commits, ~393 KB, all branches available locally (`--log-opts="--all"`)

## Result

**No leaks found (0 findings).**

No API keys, access tokens, private keys, database credentials, or other
high-confidence secrets were detected in any commit.

## Demo credentials (not leaked secrets)

The repository intentionally contains non-production demo values in demo/E2E
configuration and scripts:

- `fleet.secret: test-secret` (E2E script)
- `POSTGRES_PASSWORD=edge_sync` (E2E / compose / CLI defaults)
- `EDGE_SYNC_FLEET_SECRET=change-me`, `EDGE_SYNC_ADMIN_TOKEN=change-me` (`.env.example` placeholders)

These are not secrets and are not treated as such; they are clearly documented
demo/placeholder values. They must never be reused in a real deployment.

## False-positive handling

Zero findings → no false positives to triage.

## Remaining risks / recommendations

1. The scan covers history available **locally**. Re-run in CI after pushing so
   any future secret is caught pre-merge (see recommendation 2).
2. Add a Gitleaks job to CI (a `gitleaks detect` step) as a prevention measure.
3. Rotate the GitHub Personal Access Token used by `origin` if it was ever
   committed to a private tooling repository elsewhere; it is not present here.

No history rewrite is required (no findings).
