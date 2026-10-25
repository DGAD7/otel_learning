#!/bin/bash

set -e

root_path=$(git rev-parse --show-toplevel)

docker build \
	--platform linux/arm64 \
	--tag "otel-pi" \
	"$root_path/docker"
