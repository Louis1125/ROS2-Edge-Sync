# Pinned, reproducible Drogon build image for the control plane.
FROM ubuntu:22.04

ARG DROGON_REF=df418bfdd99d4cc1ebd8598b8ced4f74d4f488ce
ARG DROGON_PREFIX=/opt/drogon

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    build-essential cmake git ca-certificates \
    libjsoncpp-dev uuid-dev zlib1g-dev libssl-dev libbrotli-dev \
    libsqlite3-dev libpq-dev \
    && rm -rf /var/lib/apt/lists/*

RUN git clone --depth 1 --recurse-submodules https://github.com/drogonframework/drogon.git /opt/drogon-src \
    && cd /opt/drogon-src \
    && git checkout "${DROGON_REF}" \
    && cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="${DROGON_PREFIX}" \
    && cmake --build build -j"$(nproc)" \
    && cmake --install build \
    && rm -rf /opt/drogon-src
