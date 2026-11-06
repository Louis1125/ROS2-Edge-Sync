# Roadmap

- **v0.1.0-alpha.1** — clean Docker install via `./edge-sync up`, two-robot sync
  demo, reproducible tests, documented known limitations.
- **v0.1.0-rc.1** — real K3s/kind deployment validation, essential UI (overview,
  agents, sync jobs, deployment wizard), integration + recovery tests.
- **v0.1.0** — reproducible install + basic deployment, security review, complete
  operator documentation.

Current status: the data plane, durable command dispatch, control plane
(PostgreSQL + bearer auth + heartbeat), CLI, and core frontend workflows are
implemented and tested. Remaining before Alpha: a from-clean-install pass and
k3d/kind cluster validation.
