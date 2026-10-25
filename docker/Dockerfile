FROM debian:trixie

RUN apt-get update && apt-get install -y --no-install-recommends \
	build-essential \
	cmake \
	ninja-build \
	git \
	ca-certificates \
	pkg-config \
	curl \
	zip \
	unzip \
	tar \
	perl \
	python3 \
	linux-libc-dev \
	&& rm -rf /var/lib/apt/lists/*

ENV VCPKG_ROOT=/opt/vcpkg
ENV VCPKG_FORCE_SYSTEM_BINARIES=1

RUN git clone https://github.com/microsoft/vcpkg.git "$VCPKG_ROOT" \
	&& git -C "$VCPKG_ROOT" checkout 2026.10.16 \
	&& "$VCPKG_ROOT/bootstrap-vcpkg.sh" -disableMetrics

WORKDIR /src
