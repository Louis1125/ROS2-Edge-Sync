# Security Policy

## Reporting a vulnerability

Please report security issues privately rather than opening a public issue.
Open a private security advisory via GitHub, or contact the maintainers
directly. We will acknowledge within 72 hours and aim to triage within one week.

## Security model

ROS2-Edge-Sync transfers configuration, maps and software artifacts between
robots. The security model separates two concerns:

1. **Robot middleware communication** — ROS 2 DDS (local nodes, lifecycle).
2. **Distributed artifact transfer** — the Edge-Sync Connection Foundation,
   which is the only path for cross-robot data movement and is expected to use
   TLS + authentication (M6).

Do not ship large artifacts through ROS 2 topics; use the connection layer.

## Current status

As of M0 the repository contains scaffolding only. TLS, authentication and
authorization are designed for M6 and are **not yet implemented**. Do not deploy
the current code to production or to untrusted networks.

See [docs/SECURITY.md](docs/SECURITY.md) for the threat model.
