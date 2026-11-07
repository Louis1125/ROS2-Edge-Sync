# Installation

Three deployment profiles, all behind the `./edge-sync` CLI.

## Docker (local development / demo)

Prerequisites: Docker.

```bash
git clone https://github.com/Louis1125/ROS2-Edge-Sync.git && cd ros2-edge-sync
./edge-sync up --mode local       # builds/pulls images, migrates, waits for health
./edge-sync status                # service states, URLs, robots, jobs
./edge-sync down                  # stop (preserves data; --reset deletes it)
```

Web console at http://localhost:5173 (API at :8080).

## Build from source (ROS 2 Humble)

Prerequisites: ROS 2 Humble, C++20 toolchain, colcon, protobuf, Boost, SQLite3, GTest.

```bash
source /opt/ros/humble/setup.bash
rosdep install --from-paths src --ignore-src -r -y
colcon build && colcon test
```

## K3s / Kubernetes (edge + production)

Prerequisites: a K3s/Kubernetes cluster, kubectl, Helm.

```bash
./edge-sync up --mode edge              # installs via Helm (single edge server)
./edge-sync up --mode production        # Kubernetes + external PostgreSQL + TLS
```

Validated on k3d — see [docs/release/K3S_ACCEPTANCE.md](docs/release/K3S_ACCEPTANCE.md).

## Robot agent

```bash
edge_syncd --config config/robot.yaml                              # run an agent
edge_syncd --config config/robot.yaml --seed /data/map.bin --artifact map-1   # publish an artifact
```

Agents connect outbound to the control-plane gateway; they do not need to join
the Kubernetes cluster. See [COMMANDS.md](COMMANDS.md) for configuration keys.
