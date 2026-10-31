# Security

## Threat model

Assets: configuration, maps, task descriptions and software versions shipped
between robots. Threats include interception, tampering, impersonation and
replay over the transfer channel, plus unauthorized access to stored artifacts.

## Controls (target state, M6)

- **Transport security** — TLS on the Asio transport (`edge_sync_transport_asio`).
- **Authentication** — robot identity + public-key identity
  (`RobotIdentity::public_key_id`).
- **Integrity** — SHA-256 (optionally BLAKE3) content hashing; every chunk and
  manifest is verified before activation.
- **Atomic activation** — new versions are staged then activated atomically so a
  torn transfer never replaces a live artifact.
- **Least privilege** — the coordinator and robots run with minimal scopes.

## Current status

M0 scaffolding only. Transport security and authentication are not implemented
yet; see SECURITY.md at the repo root for reporting guidance.
