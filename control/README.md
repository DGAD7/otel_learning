# Controller (C++)

`led_control` is a small gRPC server that runs on a Raspberry Pi and switches an LED on or off. The [UI](../ui/README.md) calls it over the network, and it sends logs, metrics and traces through OpenTelemetry.

```
UI (Go) ──gRPC SetLed──► led_control ──GPIO 17──► LED
                              │
                              └── OTLP ──► collector on the Pi
```

## Hardware

| Part | Connected to |
|---|---|
| LED long leg (+) | 220–330 Ω resistor, then pin 11 (GPIO17) |
| LED short leg (−) | Pin 9 (GND) |

The GPIO chip (`0`) and line (`17`) are set in [main.cpp](main.cpp). Run `pinout` on the Pi to see the header layout.

## Code layout

| Path | Content |
|---|---|
| [main.cpp](main.cpp) | Starts telemetry, the LED and the gRPC server |
| [led_control.cpp](led_control.cpp) | The gRPC service: handles `SetLed`, creates the server span, logs and counts switch-ons |
| [led/](led/) | Switches the GPIO pin through the Linux GPIO character device (`/dev/gpiochip0`) |
| [telemetry/](telemetry/) | Sets up OpenTelemetry logging, metrics and tracing |
| [grpc/](grpc/) | Generates the C++ gRPC code from [proto/led_control.proto](../proto/led_control.proto) during the build |
| [3rd_party/](3rd_party/) | Downloads opentelemetry-cpp with CMake FetchContent |

## Building for the Raspberry Pi

The controller uses the Linux GPIO API, so it is built for Linux. The build runs on the development machine inside a Debian arm64 container, which produces a binary for the Pi (64-bit Raspberry Pi OS).

gRPC, Protobuf and their dependencies come from [vcpkg](https://vcpkg.io) (listed in [vcpkg.json](../vcpkg.json)) and are linked statically. The result is a single file that needs nothing else installed on the Pi.

From the repository root:

```sh
./scripts/build_docker_image.sh   # once, or after changing docker/Dockerfile
./scripts/build_aarch64.sh        # configure and build
```

The binary is written to `build-pi/control/led_control`.

The first build takes a while because vcpkg compiles gRPC. The compiled libraries are cached in `.vcpkg-cache/`, so later builds are fast.

### Cross-compilation issues

- **macOS binaries don't run on Linux.** Building in a Debian arm64 container produces a Linux binary for the Pi.
- **Generated protobuf code must match the Protobuf version exactly.** It is now generated during each build instead of being committed.
- **Debian's gRPC is dynamic only.** The Pi needed extra `.so` files, including OpenTelemetry's `libopentelemetry_proto_grpc`.

vcpkg solves the last point: it builds gRPC and its dependencies as static libraries at pinned versions, so `led_control` is a single file.

vcpkg isn't required to run on the Pi: Debian's dynamic packages work too. But then the Pi needs the matching runtime packages installed, plus `libopentelemetry_proto_grpc.so` copied and found via `LD_LIBRARY_PATH`. With vcpkg, deploying means copying one binary.

## Deploying and running

Copy the binary to the Pi:

```sh
scp build-pi/control/led_control <user>@<pi-ip>:~/
```

Run it on the Pi:

```sh
OTEL_EXPORTER_OTLP_ENDPOINT=http://localhost:4317 ./led_control
```

Stop it with Ctrl+C. The remaining telemetry is sent before the program exits. 

### Settings

| Environment variable | Default | Meaning |
|---|---|---|
| `LED_CONTROL_LISTEN_ADDRESS` | Hardcoded to `0.0.0.0:50051` | Address and port of the gRPC server |
| `OTEL_EXPORTER_OTLP_ENDPOINT` | not set | Where to send telemetry. If not set, logs are only printed to the console and no traces or metrics are exported. |

The collector on the Pi listens on `localhost:4317`. See the [collector README](../collector/README.md) for its setup.

## Telemetry

All telemetry uses `service.name = led-control`.

| Signal | What is sent |
|---|---|
| Logs | Startup messages and `change the state of LED` for every call |
| Metrics | `led.switch_on`: counter of switch-ons (`led_switch_on_total` in Mimir) |
| Traces | A server span `ledcontrol.LedControl/SetLed` for every call. It continues the trace started by the UI, so both appear as one trace in Tempo. |

## OpenTelemetry for C++

* Documentation: https://github.com/open-telemetry/opentelemetry-cpp
* This project includes opentelemetry-cpp with CMake FetchContent, as described in https://github.com/open-telemetry/opentelemetry-cpp/blob/main/INSTALL.md
