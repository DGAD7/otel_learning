#!/bin/bash

set -e
set -x

root_dir=$(git rev-parse --show-toplevel)

DOCKER_ARGS=(run
	--rm
	--platform linux/arm64
	--volume "$root_dir:/src"
	--workdir /src
	otel-pi
)

docker "${DOCKER_ARGS[@]}" \
	sh -c "cmake -S . -B build-pi -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build build-pi"
