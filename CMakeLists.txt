cmake_minimum_required(VERSION 3.20)

# This repository is a colcon workspace, not a single CMake project.
# Each ROS 2 package under src/ carries its own CMakeLists.txt + package.xml.
# Build the whole workspace from the repository root with:
#   colcon build
#
# This file exists only so that tooling which expects a top-level CMakeLists.txt
# (IDEs, language servers) has a stable anchor.
project(ros2_edge_sync_workspace NONE)

message(STATUS "ROS2-Edge-Sync: build with 'colcon build' from the repository root")
