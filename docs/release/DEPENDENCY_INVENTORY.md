# Dependency Inventory

Precise direct-dependency inventory. Machine-readable SBOM: `sbom.spdx.json`.

## 1. Direct C++ / native dependencies

| Name | Version | Source | SPDX | Usage | Attribution |
|---|---|---|---|---|---|
| Boost (Asio/System) | 1.74 (Humble toolchain) | boost.org | BSL-1.0 | installed (headers) | none beyond license |
| Protocol Buffers | 3.21 (Ubuntu 22.04) | protobuf | BSD-3-Clause | installed | none |
| SQLite3 | 3.37 (Ubuntu 22.04) | sqlite.org | public domain | installed | none |
| OpenSSL | 3.x | openssl.org | Apache-2.0 | installed | none |
| Drogon | pinned `df418bf` | drogonframework/drogon | MIT | built from source | none |
| trantor (Drogon submodule) | pinned `ae1bdf6` | an-tao/trantor | BSD-3-Clause | built from source | none |
| jsoncpp | 1.9 (Ubuntu 22.04) | jsoncpp | MIT | installed | none |
| zlib | 1.2 | zlib.net | Zlib | installed | none |
| brotli | 1.0 | brotli | MIT | installed | none |
| libuuid | util-linux | kernel.org | BSD-3-Clause | installed | none |
| libpq | 14 | postgresql.org | PostgreSQL | installed | none |
| GoogleTest | 1.11 | google/googletest | BSD-3-Clause | test-only | none |

## 2. ROS 2 (Humble) dependencies

Direct `<depend>` packages — all resolved from the ROS 2 Humble distro:

`rclcpp`, `rclcpp_action`, `rclcpp_lifecycle`, `lifecycle_msgs`,
`rosidl_default_generators`, `builtin_interfaces`, `std_msgs`, `ament_cmake`,
`gtest_vendor`, `rmw_fastrtps_cpp`.

These core packages are **Apache-2.0**. RMW implementations carry their own
licences and are relevant only if redistributed:
- Fast DDS (eProsima): Apache-2.0
- Cyclone DDS: Eclipse Public License 2.0 (not a direct dependency)

## 3. Frontend (npm) dependencies

| Name | Version | Source | SPDX | Usage |
|---|---|---|---|---|
| vite | 5.4.10 | vitejs/vite | MIT | dev |
| typescript | 5.6.3 | microsoft/TypeScript | Apache-2.0 | dev |

Transitive (bundled by Vite): rollup (MIT), esbuild (MIT) — dev-only, not
shipped in the runtime.

## 4. Docker base images

| Image | Source | Notes |
|---|---|---|
| ubuntu:22.04 | Docker Hub | per-package licenses |
| node:20-alpine | Docker Hub | Node (MIT) + Alpine packages |
| postgres:16-alpine | Docker Hub | PostgreSQL License |
| osrf/ros:humble-desktop | Docker Hub | ROS (Apache-2.0) + Ubuntu; **Gazebo/LGPL components unresolved** |
| alpine/helm | Docker Hub | Apache-2.0 |
| edge-sync-drogon-base | this repo | derived from ubuntu:22.04 + Drogon |

## 5. Build / development tools

cmake, colcon, clang-format, protobuf-compiler, Docker, Helm, k3d, Node.js 20 —
build-time tooling, not redistributed runtime components.

## 6. Transitive with outstanding verification

- **trantor** — resolved to BSD-3-Clause (license read from the `License` file
  at pinned SHA `ae1bdf6`).
- **osrf/ros:humble-desktop** — the desktop image includes Gazebo and other
  components whose individual licenses (LGPL/GPL) are not individually
  audited here; only relevant if that image is redistributed.
- Full transitive closure of Ubuntu/ROS packages is not individually enumerated.

## Verification sources

- Licenses for Drogon and trantor were read from their upstream
  `LICENSE`/`License` files at the pinned revisions (2026-11-07).
- Other SPDX identifiers are from the respective project documentation and are
  not independently re-verified line-by-line.
