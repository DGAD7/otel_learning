# Telemetry

How `led_control` sets up OpenTelemetry logs, metrics and traces.

## CMake

[3rd_party/opentelemetry/CMakeLists.txt](../3rd_party/opentelemetry/CMakeLists.txt) fetches opentelemetry-cpp with the OTLP gRPC exporter enabled:

```cmake
set(WITH_OTLP_GRPC ON CACHE BOOL "" FORCE)
FetchContent_Declare(opentelemetry-cpp
    GIT_REPOSITORY https://github.com/open-telemetry/opentelemetry-cpp.git
    GIT_TAG        v1.29.0)
FetchContent_MakeAvailable(opentelemetry-cpp)
```

[CMakeLists.txt](CMakeLists.txt) builds a `telemetry` library:

```cmake
target_link_libraries(telemetry
PUBLIC
    opentelemetry-cpp::api                  # recording: loggers, meters, tracers
PRIVATE
    opentelemetry_logs    
    opentelemetry_exporter_ostream_logs 
    opentelemetry_exporter_otlp_grpc_log
    opentelemetry_trace   
    opentelemetry_exporter_otlp_grpc
    opentelemetry_metrics 
    opentelemetry_exporter_otlp_grpc_metrics)
```

Only the API is `PUBLIC`. The rest of the program records telemetry through the API, and the SDK and exporters stay hidden inside this library.

## Wrappers

Each signal has an `init_*()` and a `shutdown_*()` function. They are needed because:

- **Setup lives in one place.** `main.cpp` only calls `init_logging()`, `init_metrics()` and `init_tracing()` at startup, and the matching `shutdown_*()` at exit.
- **It is switchable.** OTLP export is only set up when `OTEL_EXPORTER_OTLP_ENDPOINT` is set ([otlp.h](otlp.h)). Without it, no provider is installed and every call is a no-op.
- **No data is lost.** `shutdown_*()` calls `ForceFlush()` and `Shutdown()`, so the last batch is sent before exit.

Every `init_*()` follows the same steps: **exporter → batching → provider (with `service.name`) → set as global**.

### What a provider is

The API is what the code calls (logger, meter, tracer). The **provider** is the SDK object behind it that decides where the data goes: it holds the resource (`service.name`), the batching and the exporter, and hands out loggers, meters or tracers that share them. Until `init_*()` sets a real provider as global, the API uses a no-op one, so every call does nothing. The provider also owns the export threads, which is why `g_provider` is kept for `shutdown_*()`.

### Why these files

opentelemetry-cpp has **no built-in `init_logging()`** or automatic setup from environment variables, unlike the Java agent. Without a provider, the API only returns no-op loggers, meters and tracers, so the program has to build the providers itself. `init_logging()`, `init_metrics()` and `init_tracing()` are this project's own functions for that.

| File | Why it exists |
|---|---|
| `logging.cpp`, `metrics.cpp`, `tracing.cpp` | Hold the SDK setup and the provider (`g_provider`), which `shutdown_*()` needs later. Only these files include SDK and exporter headers. |
| `logging.h`, `metrics.h`, `tracing.h` | The interface for the rest of the program: `init_*`, `shutdown_*`, and short helpers (`get_logger`, `get_tracer`, `With`) instead of the long `Provider::Get...Provider()->Get...()` chains. They include only API headers, so `main.cpp` and `led_control.cpp` compile without the SDK. |
| `custom_exporter.cpp` | Prints logs to the console as one readable line (`15:04:05.123 INFO [led_control] message key=value`). The built-in ostream exporter prints a multi-line block per record, which is hard to read in a terminal. |
| `otlp.h` | `otlp_enabled()`: the single check whether `OTEL_EXPORTER_OTLP_ENDPOINT` is set, shared by all three `init_*()`. |

| File | Signal | Exporter / batching | Helpers |
|---|---|---|---|
| [logging.cpp](logging.cpp) | Logs | Console ([custom_exporter](custom_exporter.cpp)) always; OTLP with `BatchLogRecordProcessor` | `get_logger(name)`, `With({...})` for attributes |
| [metrics.cpp](metrics.cpp) | Metrics | OTLP with `PeriodicExportingMetricReader` (every 10 s) | – |
| [tracing.cpp](tracing.cpp) | Traces | OTLP with `BatchSpanProcessor`; also installs the W3C propagator | `get_tracer(name)` |

Usage:

```cpp
get_logger("led_control")->Info("change the state of LED", With({{"new state", on}}));

auto meter = opentelemetry::metrics::Provider::GetMeterProvider()->GetMeter("led_control");
auto counter = meter->CreateUInt64Counter("led.switch_on");   // once
counter->Add(1);                                              // per call
```

## Continuing the trace from the UI

The UI and `led_control` are separate processes, so the trace context has to travel with the gRPC call.

1. **UI (Go):** `otelgrpc.NewClientHandler()` starts a client span and, with the `TraceContext` propagator, writes it into the gRPC metadata as a `traceparent` header.
2. **Controller:** `init_tracing()` installs the matching W3C `HttpTraceContext` propagator.
3. **`SetLed`** in [led_control.cpp](../led_control.cpp) reads the header and uses it as the parent of its server span:

```cpp
grpc_server_carrier carrier{context};        // reads keys from client_metadata()
auto parent = propagator->Extract(carrier, RuntimeContext::GetCurrent());

StartSpanOptions options;
options.kind   = SpanKind::kServer;
options.parent = GetSpan(parent)->GetContext();

auto span  = tracer->StartSpan("ledcontrol.LedControl/SetLed", {...}, options);
auto scope = tracer->WithActiveSpan(span);   // logs inside get this trace ID
```

`grpc_server_carrier` is a small adapter that lets the propagator read gRPC metadata. Both sides must use the same format (W3C), or the controller starts a new, unconnected trace.
