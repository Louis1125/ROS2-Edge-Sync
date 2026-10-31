#!/usr/bin/env bash
set -eo pipefail

if [ -f "/opt/ros/${ROS_DISTRO}/setup.bash" ]; then
  # shellcheck disable=SC1091
  source "/opt/ros/${ROS_DISTRO}/setup.bash"
fi
if [ -f "/opt/edge-sync/setup.bash" ]; then
  # shellcheck disable=SC1091
  source "/opt/edge-sync/setup.bash"
fi

exec "$@"
