FROM debian:trixie

RUN apt-get update && apt-get install -y --no-install-recommends \
	build-essential \
	cmake \
	ninja-build \
	git \
	ca-certificates \
	pkg-config \
	libgrpc++-dev \
	libprotobuf-dev \
	protobuf-compiler \
	protobuf-compiler-grpc \
	libabsl-dev \
	&& rm -rf /var/lib/apt/lists/*

WORKDIR /src
