# otel_learning
learn otel

### Pre-requisites
#### Go Protobuf and grpc
```
go install google.golang.org/protobuf/cmd/protoc-gen-go@latest
go install google.golang.org/grpc/cmd/protoc-gen-go-grpc@latest
export PATH="$PATH:$(go env GOPATH)/bin"
```
#### C++ protobuf and grpc
```
brew install protobuf grpc
```

#### Docker 
```
brew install docker-compose
brew install colima (macos)
```

## Architecture

```
 sources                 pipeline                  backends                 frontend
┌──────────────┐      ┌───────────────┐      ┌──────────────────┐      ┌───────────┐
│ led_control  │      │ agent (Pi)    │      │ Loki   (logs)    │      │           │
│ UI (Go)      │ ───► │ gateway (Mac) │ ───► │ Tempo  (traces)  │ ◄─── │  Grafana  │ ◄── browser
└──────────────┘      └───────────────┘      │ Mimir  (metrics) │      │           │
  produce data          carry & route        └──────────────────┘      └───────────┘
                                               store & answer queries    show data
```

| Role | Components | Job |
|---|---|---|
| Sources | `led_control` (C++), UI (Go) | Produce logs, traces and metrics through the OpenTelemetry SDKs |
| Pipeline | Agent collector on the target, gateway collector on the development machine | Receive, enrich, batch and route telemetry. They neither store nor display it. |
| Backends | Loki, Tempo, Mimir | Store telemetry and answer queries (LogQL, TraceQL, PromQL) |
| Frontend | Grafana | Displays telemetry. It stores none itself; it queries the backends through its data sources. |

Because each role only talks to its neighbours, a backend can be replaced (for example Mimir with Prometheus) without changing the applications or the target's collector.

"Frontend" and "backend" here refer to the observability stack. Within the LED application itself, the UI is the frontend and `led_control` the backend; in the diagram above both are sources.

See the [collector README](collector/README.md) for ports, configuration and versions.

## Components
- [UI (Go)](ui/README.md)
- [Controller (C++)](control/README.md)
- [Collector](collector/README.md)

