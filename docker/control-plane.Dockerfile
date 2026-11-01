FROM edge-sync-drogon-base
WORKDIR /workspace
COPY . /workspace
RUN cmake -B build/control -S apps/edge_sync_control_plane \
      -DCMAKE_PREFIX_PATH="/opt/drogon;/workspace/install" \
      -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build/control -j"$(nproc)"
