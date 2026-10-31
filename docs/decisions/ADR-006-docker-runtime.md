# ADR-006 — Docker runtime

- **Status:** accepted
- **Date:** 2026-11-01

## Context

A reproducible development, test and deployment environment is required for a
ROS 2 C++ project.

## Decision

Adopt Docker as the standard environment: one development image and one
runtime image built from the same compiled code, with role configured per
container (robot/coordinator) rather than per-module containers. Use a Fast DDS
Discovery Server for multi-container ROS 2 discovery.

## Consequences

- `ROS_DISTRO` is a build arg (default `jazzy`).
- Multi-stage builds produce a minimal runtime image.
- Each robot runs the same binary with a different config.
