# Debugging

## Sanitizers

Build with sanitizers and run tests:

```bash
colcon build --cmake-args \
  -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined"
colcon test
```

ThreadSanitizer for connection/session code:

```bash
colcon build --cmake-args \
  -DCMAKE_CXX_FLAGS="-fsanitize=thread -fno-omit-frame-pointer" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=thread"
```

## GDB

```bash
gdb --args ros2 run edge_sync_runtime edge_syncd --config config/robot.yaml
```

## Logging

Set `logging.level: trace` in the config file, or `RCLPY`/spdlog env vars, to
get verbose output. See `edge_sync_base/logger.hpp`.
