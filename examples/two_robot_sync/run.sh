#!/usr/bin/env bash
# Two-robot vertical slice: robot-a listens; robot-b connects and both complete
# the handshake. Run inside the dev container after `colcon build`.
set -eo pipefail
cd "$(dirname "$0")"

export PATH=/workspace/install/edge_sync_runtime/bin:$PATH
export LD_LIBRARY_PATH=$(find /workspace/install -maxdepth 2 -type d -name lib | tr '\n' ':')$LD_LIBRARY_PATH

echo "=== starting robot-a (listener) ==="
edge_syncd --config "$PWD/robot-a.yaml" > /tmp/robot-a.log 2>&1 &
A=$!

sleep 1
echo "=== starting robot-b (connects to robot-a) ==="
edge_syncd --config "$PWD/robot-b.yaml" > /tmp/robot-b.log 2>&1 &
B=$!

sleep 3
echo; echo "===== robot-a log ====="; cat /tmp/robot-a.log
echo; echo "===== robot-b log ====="; cat /tmp/robot-b.log

echo; echo "=== graceful shutdown (SIGINT) ==="
kill -INT $A $B
sleep 1
wait $A $B 2>/dev/null || true
echo "both processes exited"
