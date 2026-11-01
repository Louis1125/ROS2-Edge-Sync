# Security Policy

## Reporting

Report vulnerabilities privately (GitHub private advisory or direct to
maintainers), never as a public issue. We aim to acknowledge within 72 hours.

## Implemented

- **Authenticated robot handshake** — robots present `HMAC-SHA256(shared_secret,
  robot_id)` during the handshake; the server rejects peers that fail to
  verify. Configure the fleet secret via `set_shared_secret` / config.
- **Integrity** — per-chunk and full-artifact SHA-256 verification before
  atomic activation; corrupted or incomplete artifacts never become active.

## Not yet implemented (known gaps)

- **TLS on the wire** — the Asio transport is plaintext. Deploy behind a VPN /
  mTLS tunnel until transport encryption lands.
- **Per-robot enrollment secrets** — currently a single shared fleet secret;
  expiring single-use enrollment credentials are planned.
- **Control-plane API authentication** — the HTTP API has no auth yet; do not
  expose it publicly.
- **Replay protection** — the handshake token is static; a challenge-response
  or certificate-based scheme is the hardening path.

Do not deploy to untrusted networks or production until these are resolved.
