# ADR-001 — Modular architecture

- **Status:** accepted
- **Date:** 2026-11-01

## Context

The system must synchronize state and artifacts across a fleet of ROS 2 robots
over unreliable networks, while remaining reusable as a developer
infrastructure product.

## Decision

Organize the system as a C++20 modular monolith: a single deployable program
composed of independently compiled, tested and reusable libraries, layered into
Base/Connection foundations, domain, application, infrastructure and adapters.

## Consequences

- Clear module contracts and dependency direction (see ARCHITECTURE.md).
- ROS 2 can be replaced without rewriting the core.
- Rejects premature microservices: packages become services only when a feature
  genuinely needs independent deployment.
