# Metrics

## Integration Guide

1. Define your metrics with the `METRICS_DEFINE` macro in a file. e.g. [metrics.def](metrics.def)
2. Let the compiler know where your file is located using `METRICS_USER_DEFINES`
   macro[^1]. e.g. [`-DMETRICS_USER_DEFINES=\"src/my_metrics.def\"`](https://github.com/onkwon/libmcu/blob/master/project/runner.mk#L10)
3. Call `metrics_init()` once at startup
4. Update metric values with the set APIs throughout your code
5. Periodically call `metrics_collect()` to encode and transmit the current snapshot, then `metrics_reset()` to clear for the next interval. Use `metrics_collect_reset()` when collect and reset must be atomic.

### Metric Types

| Macro | Description |
| ----- | ----------- |
| `METRICS_DEFINE(key)` | Untyped (backward-compatible) |
| `METRICS_DEFINE_COUNTER(key)` | Monotonically increasing integer |
| `METRICS_DEFINE_GAUGE(key, min, max)` | Arbitrary value with range hint |
| `METRICS_DEFINE_PERCENTAGE(key)` | Ratio in [0, 100] |
| `METRICS_DEFINE_TIMER(key, unit)` | Elapsed time; `unit`: `ms`/`MS`, `us`/`US`, `s`/`S` |
| `METRICS_DEFINE_BYTES(key)` | Byte-count value |
| `METRICS_DEFINE_BINARY(key)` | Binary value in [0, 1]; `unset` remains "no sample" |
| `METRICS_DEFINE_STATE(key)` | Categorical / enum-like state value |

[^1]: The default file name is `metrics.def`. You don't need to specify the file
location with `METRICS_USER_DEFINES` when you use the default file name and the
file is in the include path.

When using the Zephyr west module, `app/include/metrics.def` is automatically searched; for ESP-IDF components, `main/include/metrics.def` is searched by default. If you use a different location, you need to set `METRICS_USER_DEFINES` or `METRICS_USER_DIR`.

### Encoding

You can implement your own encoder using `metrics_encode_header()` and
`metrics_encode_each()`. No encoder by default, meaning just a simple byte
stream.

A ready-made [CBOR encoder](../../ports/metrics/cbor_encoder.c) is available
under `ports/metrics/`. It encodes metrics as a CBOR map with device metadata
(serial number, timestamp, version). Link the file and the
[libcbor](https://github.com/libmcu/cbor) dependency to use it.

To get the exact encoded payload size before allocation, call
`metrics_collect(NULL, 0, NULL)`.

### Usage

#### Initialization

Call `metrics_init()` once at startup. Pass `true` to force re-initialization.

```c
metrics_init(false);
```

#### Setting Values

Use `metrics_set()` to record a value. Use `metrics_increase()` /
`metrics_increase_by()` for counters. Min/max helpers keep extreme values
across updates.

```c
metrics_set(BatteryPCT, 87);
metrics_increase(TxPackets);
metrics_increase_by(RxBytes, len);
metrics_set_if_max(PeakRSSI, rssi);
metrics_set_pct(CpuUsagePCT, busy_ticks, total_ticks);
```

#### Collecting

`metrics_collect()` encodes all current values into a caller-supplied buffer.
Call it with `NULL` / `0` first to perform a dry run and determine the exact
size needed.

```c
size_t len = metrics_collect(NULL, 0, NULL); /* dry run to get size */
uint8_t buf[len];
metrics_collect(buf, len, NULL);
/* transmit buf over your transport of choice */
metrics_reset(); /* clear values for the next interval */
```

`metrics_collect_reset()` performs collect and reset in the same lock section.
This function does not support dry runs. If a `NULL` buffer or insufficient
buffer is provided, it returns 0 and leaves metric values unchanged.
Because it performs an internal sizing pass, encoders must handle `buf == NULL`
without side effects.

```c
size_t len = metrics_collect(NULL, 0, NULL);
uint8_t buf[len];
size_t written = metrics_collect_reset(buf, len, NULL);
/* transmit buf[0..written) if written is not zero */
```

### Synchronisation

Implement `metrics_lock()` and `metrics_unlock()` in a multi-threaded
environment.

```c
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void metrics_lock(void) {
	pthread_mutex_lock(&lock);
}
void metrics_unlock(void) {
	pthread_mutex_unlock(&lock);
}
```
