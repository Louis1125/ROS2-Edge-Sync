# Security Audit — Git History

- **Date:** 2026-11-07
- **Scanner:** Gitleaks v8.30.1 (static release binary)
- **Scope:** full local git history, all fetched refs — **46 commits**, ~394 KB
  (`--log-opts="--all"` after `git fetch --all --prune --tags`)

## Result

**No leaks found (0 findings).**

No API keys, access tokens, private keys, database credentials, or other
high-confidence secrets were detected in any commit.

## Demo credentials (not leaked secrets)

The repository intentionally contains non-production demo values in demo/E2E
configuration and scripts: `fleet.secret: test-secret`,
`POSTGRES_PASSWORD=edge_sync`, and `change-me` placeholders in `.env.example`.
These are documented demo/placeholder values, not secrets.

## False-positive handling

Zero findings → no false positives to triage. No allowlist rules were created
and none are required.

## Remaining risks / recommendations

1. The scan covers history available **locally** (all fetched refs). A CI
   `gitleaks/gitleaks-action@v3` job now scans on every push/PR plus a weekly
   schedule, so future secrets are caught pre-merge.
2. Rotate the GitHub credential used by `origin` if it was ever committed
   elsewhere; it is not present in this history.
3. No history rewrite is required (no findings).
