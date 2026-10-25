#!/bin/bash

set -e
set -x

root_dir=$(git rev-parse --show-toplevel)

DOCKER_ARGS=(run
	--rm
	--platform linux/arm64
	--volume "$root_dir:/src"
	--workdir /src
	--env VCPKG_DEFAULT_BINARY_CACHE=/src/.vcpkg-cache
	otel-pi
)

CMAKE_CONFIGURE="cmake -S . -B build-pi -G Ninja \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
	-DVCPKG_TARGET_TRIPLET=arm64-linux"

mkdir -p "$root_dir/.vcpkg-cache"

docker "${DOCKER_ARGS[@]}" \
	sh -c "$CMAKE_CONFIGURE && cmake --build build-pi"
