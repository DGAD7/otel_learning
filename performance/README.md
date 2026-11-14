# Performance: with and without OpenTelemetry

Measurements of `led_control` on the Raspberry Pi 4.

| Mode | Command |
|---|---|
| With OTel | `OTEL_EXPORTER_OTLP_ENDPOINT=http://localhost:4317 ./led_control` |
| Without OTel | `./led_control` |

## Idle (no requests)

Threads shown with `top -H -p $(pidof led_control)`.

### With OTel

```
    PID USER      PR  NI    VIRT    RES    SHR S  %CPU  %MEM     TIME+ COMMAND
   2067 nobel     20   0  708344  16192  14560 S   0.0   0.4   0:00.04 led_control
   2068 nobel     20   0  708344  16192  14560 S   0.0   0.4   0:00.00 grpc_global_tim
   2069 nobel     20   0  708344  16192  14560 S   0.0   0.4   0:00.00 event_engine
   2070 nobel     20   0  708344  16192  14560 S   0.0   0.4   0:00.00 event_engine
   2071 nobel     20   0  708344  16192  14560 S   0.0   0.4   0:00.00 event_engine
   2072 nobel     20   0  708344  16192  14560 S   0.0   0.4   0:00.00 event_engine
   2073 nobel     20   0  708344  16192  14560 S   0.0   0.4   0:00.00 lifeguard
   2074 nobel     20   0  708344  16192  14560 S   0.0   0.4   0:00.00 led_control
   2075 nobel     20   0  708344  16192  14560 S   0.0   0.4   0:00.00 led_control
   2076 nobel     20   0  708344  16192  14560 S   0.0   0.4   0:00.00 led_control
   2077 nobel     20   0  708344  16192  14560 S   0.0   0.4   0:00.00 grpcpp_sync_ser
   2078 nobel     20   0  708344  16192  14560 S   0.0   0.4   0:00.00 led_control
```

### Without OTel

```
    PID USER      PR  NI    VIRT    RES    SHR S  %CPU  %MEM     TIME+ COMMAND
   2102 nobel     20   0  552368  13580  12260 S   0.0   0.3   0:00.00 led_control
   2103 nobel     20   0  552368  13580  12260 S   0.0   0.3   0:00.00 grpc_global_tim
   2104 nobel     20   0  552368  13580  12260 S   0.0   0.3   0:00.00 event_engine
   2105 nobel     20   0  552368  13580  12260 S   0.0   0.3   0:00.00 event_engine
   2106 nobel     20   0  552368  13580  12260 S   0.0   0.3   0:00.00 event_engine
   2107 nobel     20   0  552368  13580  12260 S   0.0   0.3   0:00.00 event_engine
   2108 nobel     20   0  552368  13580  12260 S   0.0   0.3   0:00.00 lifeguard
   2109 nobel     20   0  552368  13580  12260 S   0.0   0.3   0:00.00 grpcpp_sync_ser
   2110 nobel     20   0  552368  13580  12260 S   0.0   0.3   0:00.00 led_control
```

### Summary

| | Without OTel | With OTel | Difference |
|---|---|---|---|
| Threads | 9 | 12 | +3 (OTel log, span and metric workers) |
| RES (RAM in use) | 13.3 MB | 15.8 MB | +2.5 MB |
| VIRT (reserved address space) | 539 MB | 692 MB | +153 MB, reserved but not used |

## Under load with ghz

[ghz](https://github.com/bojand/ghz) calls a gRPC method repeatedly and reports latency and throughput.

### Install (on the Pi)

Download the Linux arm64 archive from the [releases page](https://github.com/bojand/ghz/releases), then:

```sh
tar xzf ghz-linux-arm64.tar.gz
sudo mv ghz /usr/local/bin/
```

Copy the proto file to the Pi (from the repository root):

```sh
scp proto/led_control.proto <user>@<pi-ip>:~/
```

### Run

Run ghz on the Pi against `localhost`, so network latency does not affect the result:

```sh
ghz --insecure --proto ~/led_control.proto \
    --call ledcontrol.LedControl/SetLed \
    -d '{"on": true}' \
    -n 20000 -c 4 --skipFirst 1000 \
    localhost:50051
```

| Option | Meaning |
|---|---|
| `-n` / `-z` | Number of calls, or duration (e.g. `-z 30s`) |
| `-c` | Calls in parallel |
| `--rps` | Limit calls per second |
| `--skipFirst` | Exclude warm-up calls from the statistics |
| `-O html -o report.html` | Save a report with charts |

Record `Requests/sec`, `Average` and the 95 % / 99 % latencies. Run each mode three times with the same options and compare the averages.

## Results

ghz on core 3 (`taskset -c 3`), 60 s, `-c 1`, requests alternating on/off, CPU governor `ondemand`.

| Target rate | Mode | Achieved | Average latency | `led_control` CPU | RES |
|---|---|---|---|---|---|
| 20/s | Off | 20/s | 1.49 ms | – | 14.6 MB |
| 20/s | On | 20/s | 1.57 ms | ≈ 4 % | 17.0 MB |
| 200/s | Off | 200/s | 0.85 ms | ≈ 16 % | 14.7 MB |
| 200/s | On | 200/s | 0.93 ms | ≈ 16 % | 18.2 MB |
| 1000/s | Off | 869/s | 0.80 ms | ≈ 52 % | 14.7 MB |
| 1000/s | On | 792/s | 0.91 ms | ≈ 61 % | 19.6 MB |

CPU values are `top` snapshots (percent of one core) and only approximate.

### Summary

- OpenTelemetry (one span, one log and one counter per call) adds about **0.1 ms per call**.
- Memory grows by **2–5 MB**, rising with load as the batch queues fill.
- The overhead scales with how much telemetry is recorded. Sampling, log levels and batching control it.
