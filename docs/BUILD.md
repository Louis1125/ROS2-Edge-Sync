# Build

## Prerequisites

- ROS 2 (Humble by default; `ROS_DISTRO` is a build arg — Jazzy/Lyrical supported).
- CMake ≥ 3.20, C++20 compiler, colcon.
- protobuf, Boost (headers), SQLite3, GTest.

## Build (colcon)

```bash
source /opt/ros/$ROS_DISTRO/setup.bash
colcon build          # builds all 8 packages
colcon test           # runs unit + integration tests
colcon test-result --verbose
```

ROS-independent packages use plain CMake (no ROS needed); the ROS adapter
packages (`edge_sync_interfaces`, `edge_sync_ros2`) use ament_cmake.

## Docker (dev)

```bash
docker build -f docker/Dockerfile.dev -t edge-sync-dev --build-arg ROS_DISTRO=humble .
docker run --rm -v $PWD:/workspace -w /workspace edge-sync-dev bash -c \
  'source /opt/ros/humble/setup.bash && colcon build'
```

## Docker (runtime)

```bash
docker build -f docker/Dockerfile.runtime -t ros2-edge-sync --build-arg ROS_DISTRO=humble .
# multi-stage: builds in osrf/ros, ships a minimal ros-base image with /opt/edge-sync
```

## Two-robot demo

```bash
# inside the dev container, after colcon build:
bash examples/two_robot_sync/run.sh
```
