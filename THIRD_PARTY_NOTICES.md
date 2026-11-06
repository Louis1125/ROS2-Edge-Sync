# Third-Party Notices

ROS2-Edge-Sync is licensed under MIT. This file lists direct third-party
dependencies. Transitive dependencies are summarized; complete listings live in
each dependency's own repository.

## C++ / native

| Dependency | Version | License |
|---|---|---|
| Boost (Asio/System, headers) | 1.74 (Humble toolchain) | Boost Software License 1.0 |
| Protocol Buffers (protoc + libprotobuf) | 3.21 (Ubuntu 22.04) | BSD-3-Clause |
| SQLite3 | 3.37 (Ubuntu 22.04) | Public domain |
| OpenSSL | 3.x | Apache-2.0 |
| Drogon | pinned commit `df418bf` | MIT |
| trantor (bundled with Drogon) | (submodule) | permissive — verify exact text |
| jsoncpp | 1.9 (Ubuntu 22.04) | MIT |
| zlib | 1.2 | zlib License |
| brotli | 1.0 | MIT |
| libuuid (util-linux) | — | BSD-3-Clause |
| libpq (PostgreSQL client) | 14 | PostgreSQL License |
| GoogleTest (gtest_vendor / libgtest) | 1.11 | BSD-3-Clause |

## ROS 2 (Humble)

`rclcpp`, `rclcpp_action`, `rclcpp_lifecycle`, `lifecycle_msgs`,
`rosidl_default_generators`, `builtin_interfaces`, `std_msgs`, `ament_cmake` —
all **Apache-2.0**.

## Frontend (npm)

| Dependency | Version | License |
|---|---|---|
| htmx.org | 2.0.4 | Zero-Clause BSD (0BSD) |
| vite | 5.4.10 (dev) | MIT |
| typescript | 5.6.3 (dev) | Apache-2.0 |

## Docker base images

| Image | License notes |
|---|---|
| ubuntu:22.04 | per-package (mixed; core permissive) |
| node:20-alpine | Node (MIT) + Alpine packages |
| postgres:16-alpine | PostgreSQL License |
| osrf/ros:humble-desktop | Apache-2.0 (ROS) + Ubuntu/Gazebo components |
| alpine/helm | Apache-2.0 |

## Unresolved / to verify

- Exact license text of **trantor** (bundled transitively via Drogon).
- Full transitive closure of Ubuntu/Gazebo packages in `osrf/ros:humble-desktop`
  (ROS desktop images include Gazebo, which has LGPL/GPL components — only
  relevant if that image is redistributed).
- The MIT project license does **not** automatically grant rights over these
  third-party components; each retains its own terms.

This inventory covers **direct** dependencies. A complete legal clearance
requires reviewing each dependency's full NOTICE/attribution requirements.
