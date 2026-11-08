# Third-Party Notices

ROS2-Edge-Sync is licensed under MIT. This file summarizes third-party
dependencies; the detailed, verified inventory is
[docs/release/DEPENDENCY_INVENTORY.md](docs/release/DEPENDENCY_INVENTORY.md) and
the machine-readable SBOM is [docs/release/sbom.spdx.json](docs/release/sbom.spdx.json).

## Summary by category

### C++ / native (installed or built from source)

| Dependency | Version | SPDX |
|---|---|---|
| Boost (Asio/System) | 1.74 | BSL-1.0 |
| Protocol Buffers | 3.21 | BSD-3-Clause |
| SQLite3 | 3.37 | public domain |
| OpenSSL | 3.x | Apache-2.0 |
| Drogon | pinned `df418bf` | MIT |
| trantor (Drogon submodule) | pinned `ae1bdf6` | BSD-3-Clause |
| jsoncpp | 1.9 | MIT |
| zlib | 1.2 | Zlib |
| brotli | 1.0 | MIT |
| libuuid | util-linux | BSD-3-Clause |
| libpq | 14 | PostgreSQL |
| GoogleTest | 1.11 | BSD-3-Clause (test-only) |

### ROS 2 (Humble)

`rclcpp`, `rclcpp_action`, `rclcpp_lifecycle`, `lifecycle_msgs`,
`rosidl_default_generators`, `builtin_interfaces`, `std_msgs`, `ament_cmake`,
`gtest_vendor`, `rmw_fastrtps_cpp` — **Apache-2.0**. RMW implementations carry
their own licenses (Fast DDS: Apache-2.0; Cyclone DDS: EPL-2.0, not a direct
dependency).

### Frontend (npm)

| Dependency | Version | SPDX |
|---|---|---|
| vite | 5.4.10 | MIT (dev) |
| typescript | 5.6.3 | Apache-2.0 (dev) |

### Docker base images

`ubuntu:22.04`, `node:20-alpine`, `postgres:16-alpine`, `osrf/ros:humble-desktop`,
`alpine/helm`, and the derived `edge-sync-drogon-base`.

## Unresolved / to verify

- **trantor** — resolved to BSD-3-Clause (read from the `License` file at the
  pinned submodule SHA `ae1bdf6`).
- **osrf/ros:humble-desktop** — includes Gazebo and other components whose
  individual LGPL/GPL licenses are not individually audited; only relevant if
  that image is redistributed.
- The MIT project license does **not** automatically cover third-party
  components; each retains its own terms. A complete legal clearance requires
  reviewing each dependency's NOTICE/attribution requirements.
