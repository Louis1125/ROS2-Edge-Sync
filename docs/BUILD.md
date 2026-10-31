# Build

## Prerequisites

- ROS 2 Jazzy (Ubuntu 24.04) — `ROS_DISTRO` is a build arg, default `jazzy`.
- CMake ≥ 3.20, C++20 compiler (GCC or Clang), colcon.

## Docker (recommended)

```bash
docker compose build dev
docker compose run --rm dev bash
# inside the container:
colcon build --symlink-install
colcon test && colcon test-result --verbose
```

## Native

```bash
source /opt/ros/jazzy/setup.bash
rosdep install --from-paths src --ignore-src -r -y
colcon build --symlink-install
source install/setup.bash
ros2 pkg list | grep edge_sync
```

## Build types

```bash
colcon build --cmake-args -DCMAKE_BUILD_TYPE=Release
colcon build --cmake-args -DCMAKE_BUILD_TYPE=RelWithDebInfo
```

## Running

```bash
source install/setup.bash
ros2 run edge_sync_runtime edge_syncd --config config/robot.yaml
```
