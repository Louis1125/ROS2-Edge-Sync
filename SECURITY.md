# Security Policy

## Reporting

Report vulnerabilities privately (GitHub private advisory or direct to
maintainers), never as a public issue. We aim to acknowledge within 72 hours.

## Implemented

- **Authenticated robot handshake** — robots present `HMAC-SHA256(shared_secret,
  robot_id)` during the handshake; the server rejects peers that fail to
  verify. Configure the fleet secret via `set_shared_secret` / config.
- **Control-plane API authentication** — mutating endpoints require
  `Authorization: Bearer <token>` (constant-time compare) when
  `EDGE_SYNC_ADMIN_TOKEN` is set; health and read endpoints remain open.
- **Integrity** — per-chunk and full-artifact SHA-256 verification before
  atomic activation; corrupted or incomplete artifacts never become active.

## Not yet implemented (known gaps)

- **TLS on the wire** — the Asio transport is plaintext. Deploy behind a VPN /
  mTLS tunnel until transport encryption lands.
- **Per-robot enrollment secrets** — currently a single shared fleet secret;
  expiring single-use enrollment credentials are planned.
- **Per-robot credentials** — currently a single shared fleet secret; expiring
  single-use enrollment credentials are planned.
- **Replay protection** — the handshake token is static; a challenge-response
  or certificate-based scheme is the hardening path.
- **RBAC** — API auth is a single admin token; fine-grained roles are planned.

Do not deploy to untrusted networks or production until these are resolved.
