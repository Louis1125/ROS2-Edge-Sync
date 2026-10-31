ROS_DISTRO ?= humble

.PHONY: help build test lint docker-build docker-up docker-down clean

help:
	@echo "ROS2-Edge-Sync development targets"
	@echo "  build        - colcon build the workspace"
	@echo "  test         - build and run tests"
	@echo "  lint         - run clang-format check"
	@echo "  docker-build - build the runtime image"
	@echo "  docker-up    - start the multi-robot compose environment"
	@echo "  docker-down  - stop the compose environment"
	@echo "  clean        - remove build/, install/, log/"

build:
	. /opt/ros/$(ROS_DISTRO)/setup.sh && colcon build --symlink-install

test:
	. /opt/ros/$(ROS_DISTRO)/setup.sh && colcon build --symlink-install && colcon test && colcon test-result --verbose

lint:
	find src -name '*.hpp' -o -name '*.cpp' | xargs clang-format --dry-run --Werror

docker-build:
	docker build -f docker/Dockerfile.runtime -t ros2-edge-sync:latest .

docker-up:
	docker compose up -d

docker-down:
	docker compose down

clean:
	rm -rf build install log
