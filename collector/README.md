# Collector and observability backend

Telemetry from both applications goes through two OpenTelemetry Collectors before reaching the storage backends and Grafana.

```
 Raspberry Pi (target)                          Development machine (Docker Compose)
┌──────────────────────────┐                  ┌──────────────────────────────────────────┐
│ led_control (C++)        │                  │ UI (Go) ─┐                               │
│   │ OTLP gRPC            │                  │          ▼                               │
│   ▼ localhost:4317       │    OTLP gRPC     │     otel-collector (gateway)             │
│ otelcol-contrib (agent) ─┼──────────────────┼──►    :4317 / :4318                       │
│   memory_limiter,        │  <host-ip>:4317  │       ├─ logs    ──► Loki   ─┐           │
│   resourcedetection      │                  │       ├─ traces  ──► Tempo  ─┼─► Grafana │
└──────────────────────────┘                  │       └─ metrics ──► Mimir  ─┘    :3000  │
                                              └──────────────────────────────────────────┘
```

- **Agent (target):** the application only ever sends to `localhost`. The agent adds host information (`host.name`, `os.type`) and forwards everything to the gateway, buffering and retrying if the gateway is unreachable.
- **Gateway (development machine):** the only place that knows about the backends. It routes each signal to the right backend. When a backend is added or replaced, the target configuration does not change.

## Versions

| Component | Version | Where it runs | Purpose |
|---|---|---|---|
| `otelcol-contrib` | 0.164.0 | Target (`.deb`) and gateway (Docker) | Receives, processes and forwards telemetry |
| Loki | 3.5.0 | Docker | Stores logs |
| Tempo | 2.8.0 | Docker | Stores traces |
| Mimir | 2.16.0 | Docker | Stores metrics (Prometheus-compatible) |
| Grafana | 12.0.0 | Docker | Queries and visualises all three |

Use the **same collector version** on the target and the gateway, so both interpret the configuration the same way. All image tags are pinned; avoid `latest`.

## Files

| File | Used by | Content |
|---|---|---|
| [target/config.yaml](target/config.yaml) | Target agent | Copy of `/etc/otelcol-contrib/config.yaml` on the target |
| [compose.yaml](compose.yaml) | Docker Compose | Containers, images, ports and mounted config files |
| [otel-collector.yaml](otel-collector.yaml) | Gateway collector | Receivers, exporters and the pipeline for each signal |
| [mimir/mimir.yaml](mimir/mimir.yaml) | Mimir | Minimal single-process configuration |
| [tempo/tempo.yaml](tempo/tempo.yaml) | Tempo | OTLP receiver and local storage |
| [grafana/datasources.yaml](grafana/datasources.yaml) | Grafana | Data sources for Loki, Mimir and Tempo (provisioned at startup) |

Loki uses the default configuration shipped in its image.

---

## Collector on the target

### Choosing the right package

Releases are published at [opentelemetry-collector-releases](https://github.com/open-telemetry/opentelemetry-collector-releases/releases/tag/v0.164.0). Each release contains several distributions and one file per platform:

- **Distribution:** use `otelcol-contrib`. The core `otelcol` distribution lacks components used here, such as `resourcedetection`.
- **Format:** use the `.deb` package on Raspberry Pi OS / Debian. It installs the binary, a systemd service that starts at boot, a dedicated system user, and the configuration file `/etc/otelcol-contrib/config.yaml`.
- **Architecture:** must match the target. Check it with:

  ```sh
  dpkg --print-architecture
  ```

  | Output | File to download |
  |---|---|
  | `arm64` (Raspberry Pi OS 64-bit, used in this project) | `otelcol-contrib_<version>_linux_arm64.deb` |
  | `armhf` (Raspberry Pi OS 32-bit) | `otelcol-contrib_<version>_linux_armv7.deb` |
  | `amd64` | `otelcol-contrib_<version>_linux_amd64.deb` |

  Files ending in `linux_386` are for 32-bit x86 PCs and will not install on a Raspberry Pi.

### Installing

```sh
export VERSION=0.164.0
cd /tmp
curl -LO "https://github.com/open-telemetry/opentelemetry-collector-releases/releases/download/v${VERSION}/otelcol-contrib_${VERSION}_linux_arm64.deb"
sudo apt install ./otelcol-contrib_${VERSION}_linux_arm64.deb
```

Check that the service is running:

```sh
systemctl status otelcol-contrib
otelcol-contrib --version
```

### Configuring

The configuration in [target/config.yaml](target/config.yaml):

- receives OTLP on `localhost:4317` (gRPC) and `localhost:4318` (HTTP) only, so it is not reachable from the network
- limits its own memory use with `memory_limiter`
- adds `host.name` and `os.type` with `resourcedetection`
- forwards all three signals to the gateway with the `otlp/mac` exporter

Before deploying, set the `otlp/mac` endpoint to the IP address of the machine running the gateway (`ipconfig getifaddr en0` on macOS). Use the IP address rather than a `.local` hostname, because the collector cannot resolve mDNS names. A DHCP reservation in the router keeps the address stable.

Deploy the configuration from the repository root using scp, place the file in the correct location, validate and restart the service

```sh
sudo cp /tmp/otelcol-config.yaml /etc/otelcol-contrib/config.yaml
otelcol-contrib validate --config /etc/otelcol-contrib/config.yaml
sudo systemctl restart otelcol-contrib
journalctl -u otelcol-contrib -f
```

Always run `validate` before restarting. If the service fails to start several times in a row, systemd stops retrying; run `sudo systemctl reset-failed otelcol-contrib` after fixing the configuration.

### Sending data from the application

`led_control` only exports over OTLP when `OTEL_EXPORTER_OTLP_ENDPOINT` is set:

```sh
OTEL_EXPORTER_OTLP_ENDPOINT=http://localhost:4317 ./led_control
```

Check that the target can reach the gateway:

```sh
nc -zv <host-ip> 4317
```

---

## Gateway and backends (development machine)

### Starting

From this folder:

```sh
docker compose up -d
docker compose ps
```

| Service | Port on the host | Address |
|---|---|---|
| Collector | 4317 (gRPC), 4318 (HTTP) | Receives OTLP from the UI and the target agent |
| Grafana | 3000 | http://127.0.0.1:3000 (no login) |
| Loki | 3100 | http://127.0.0.1:3100/ready |
| Tempo | 3200 | http://127.0.0.1:3200/ready |
| Mimir | 9009 | http://127.0.0.1:9009/ready |

### Which image is downloaded

Unlike on the target, no platform-specific file has to be chosen here. Every image in [compose.yaml](compose.yaml) is a multi-platform image: one tag refers to a list of builds, one per platform (`linux/amd64`, `linux/arm64`, ...). When pulling, Docker queries the daemon for the platform it runs on and downloads the matching build.

These are Linux images. On macOS the Docker daemon runs inside a Linux VM (for example Colima), so on an Apple Silicon Mac the `linux/arm64` builds are used.


### Ports and networking

Tempo also listens on port 4317 inside the Compose network. That port is not published, so it does not conflict with the collector.

Inside the Compose network, services reach each other by service name (`loki`, `tempo`, `mimir`). These names only resolve inside the network, so the target must send to the gateway's IP address, never to a backend directly.

### Gateway collector

[otel-collector.yaml](otel-collector.yaml) receives OTLP on `0.0.0.0` (inside a container, `localhost` would only be reachable from the container itself) and routes:

| Signal | Exporter | Destination |
|---|---|---|
| Logs | `otlphttp/loki` | `http://loki:3100/otlp` |
| Traces | `otlp/tempo` | `tempo:4317` (gRPC, no TLS) |
| Metrics | `otlphttp/mimir` | `http://mimir:9009/otlp` |

`otlphttp` appends the signal path (`/v1/logs`, `/v1/metrics`) to the endpoint. The gRPC `otlp` exporter takes `host:port` without a scheme.

An exporter is only used if it is listed in a pipeline. A defined but unlisted exporter is silently ignored.

### Loki (logs)

Loki 3 accepts OTLP directly. Resource attributes become labels with dots replaced by underscores, so `service.name` becomes `service_name`.

### Tempo (traces)

[tempo/tempo.yaml](tempo/tempo.yaml) receives OTLP gRPC from the collector and stores traces on local disk under `/tmp/tempo`. `/tmp` is used because the Tempo image does not run as root and may not be allowed to create other directories.

### Mimir (metrics)

Mimir runs all of its components (distributor, ingester, querier, compactor, store-gateway) in one process. [mimir/mimir.yaml](mimir/mimir.yaml) only overrides what a single instance needs:

- `multitenancy_enabled: false`, so no tenant header is required
- `replication_factor: 1` for the ingester and store-gateway; the default of 3 would make every write fail with a single instance

Mimir exposes a Prometheus-compatible query API under `/prometheus`, which is why its Grafana data source has the type `prometheus`. Metric names are converted to Prometheus style: `led.switch_on` becomes `led_switch_on_total`.

### Grafana

[grafana/datasources.yaml](grafana/datasources.yaml) is read at startup and creates the Loki, Mimir and Tempo data sources. Because they are provisioned from a file, they survive `docker compose down` and are the same on every machine.

Anonymous admin access is enabled for local development only. Do not use these `GF_AUTH_*` settings on a shared server.

Example queries in **Explore**:

| Data source | Query | Shows |
|---|---|---|
| Loki | `{service_name="led-control"}` | Logs from the controller |
| Loki | `{service_name=~".+"}` | Logs from all services |
| Mimir | `led_switch_on_total` | Raw counter since the controller started |
| Mimir | `increase(led_switch_on_total[1h])` | Switch-ons in the last hour |
| Tempo | `{ resource.service.name = "led-ui" }` | Traces started by the UI |
| Tempo | `{ span.rpc.system = "grpc" }` | All gRPC client and server spans |

### Applying changes

- **A mounted config file changed** (`otel-collector.yaml`, `mimir.yaml`, `tempo.yaml`, `datasources.yaml`): Compose does not detect changes inside mounted files.
- **`compose.yaml` changed:** run 
```
docker compose down
docker compose up -d
```

### Troubleshooting

Follow the data from source to Grafana and find the first hop where it disappears:

1. **Target agent:** `journalctl -u otelcol-contrib -f` on the target. Errors such as `Exporting failed` or `connection refused` mean the gateway is unreachable.
2. **Gateway:** `docker compose logs -f otel-collector`. The `debug` exporter prints traces and metrics as they arrive.
3. **Gateway to backend:** `docker compose logs otel-collector | grep -iE "error|fail"`.
4. **Backend:** query it directly, for example `curl -s http://127.0.0.1:3100/loki/api/v1/label/service_name/values`.
5. **Grafana:** check the time range and use **Code** mode in Explore.
