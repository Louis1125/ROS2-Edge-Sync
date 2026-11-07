# Multi-stage: data plane (ROS 2) -> control plane (Drogon) -> runtime.
ARG ROS_BASE=osrf/ros:humble-desktop

# --- Stage 1: build the C++ data plane ---
FROM ${ROS_BASE} AS datablane
USER root
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    build-essential cmake python3-colcon-common-extensions \
    libboost-system-dev libssl-dev protobuf-compiler libprotobuf-dev \
    libsqlite3-dev libgtest-dev && rm -rf /var/lib/apt/lists/*
WORKDIR /workspace
COPY src src
COPY tests tests
RUN . /opt/ros/humble/setup.sh && colcon build

# --- Stage 2: build the control plane against Drogon + the data plane ---
FROM edge-sync-drogon-base AS controlplane
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    libprotobuf-dev protobuf-compiler libboost-system-dev && rm -rf /var/lib/apt/lists/*
WORKDIR /workspace
COPY --from=datablane /workspace/install install
COPY apps/edge_sync_control_plane apps/edge_sync_control_plane
RUN cmake -B build/control -S apps/edge_sync_control_plane \
    -DCMAKE_PREFIX_PATH="/opt/drogon;/workspace/install/edge_sync_base;/workspace/install/edge_sync_core;/workspace/install/edge_sync_protocol;/workspace/install/edge_sync_connection;/workspace/install/edge_sync_connection_asio" \
    -DCMAKE_BUILD_TYPE=Release \
 && cmake --build build/control -j2

# --- Stage 3: runtime ---
FROM edge-sync-drogon-base
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    libprotobuf-dev libboost-system-dev && rm -rf /var/lib/apt/lists/*
COPY --from=datablane /workspace/install /opt/edge-sync/lib
COPY --from=controlplane /workspace/build/control/edge_sync_control_plane /opt/edge-sync/bin/edge_sync_control_plane
COPY apps/edge_sync_control_plane/migrations /opt/edge-sync/migrations
ENV LD_LIBRARY_PATH=/opt/drogon/lib:/opt/edge-sync/lib/edge_sync_base/lib:/opt/edge-sync/lib/edge_sync_core/lib:/opt/edge-sync/lib/edge_sync_protocol/lib:/opt/edge-sync/lib/edge_sync_connection/lib:/opt/edge-sync/lib/edge_sync_connection_asio/lib
ENV MIGRATIONS_DIR=/opt/edge-sync/migrations
WORKDIR /tmp
EXPOSE 8080 7443
CMD ["/opt/edge-sync/bin/edge_sync_control_plane"]
