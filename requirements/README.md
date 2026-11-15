# Where OpenTelemetry can run

OpenTelemetry has three parts, and each has different requirements.

## 1. SDK (inside the application)

The SDK is a library, so it runs wherever its language runs on an operating system.

| Language | Typical minimum |
|---|---|
| C++ | C++14 compiler, CMake; Linux, macOS or Windows. OTLP export needs gRPC or libcurl. |
| Go, Java, Python, .NET, JavaScript, Rust, ... | A supported runtime version (see each SDK's README) |

Requirements:

- An OS with threads, a clock and networking. Export and batching run in background threads.
- A few MB of RAM, plus the batch queues.

**Microcontrollers / bare metal:** there's no official SDK. These devices usually send simple data (for example over serial, MQTT or plain HTTP) to a nearby device that runs an SDK or collector.

## 2. Collector (agent or gateway)

The collector is a single Go binary, which is an optional step between the applications and the backends.

| | Supported |
|---|---|
| OS | Linux, macOS, Windows |
| CPU | amd64, arm64, armv7, and others on Linux |
| Packages | `.deb`, `.rpm`, `.tar.gz`, `.msi`, Docker images |
| Resources | Roughly 50–100 MB RAM and a small share of one core at low volume. Both grow with throughput. |

A Raspberry Pi or a similar embedded Linux board is enough for an agent.

## 3. Backend and frontend

Storage (e.g. Loki, Tempo, Mimir, Jaeger, Prometheus, or a commercial service) and a UI (e.g. Grafana). These need the most resources, so they usually run on a server, in the cloud, or on a development machine, not on the target.

## Network

Data is sent with OTLP: port **4317** (gRPC) or **4318** (HTTP).
