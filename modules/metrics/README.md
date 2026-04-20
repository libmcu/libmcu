# Metrics

## Integration Guide

1. Define your metrics with the `METRICS_DEFINE` macro in a file. e.g. [metrics.def](metrics.def)
2. Let the compiler know where your file is located using `METRICS_USER_DEFINES`
   macro[^1]. e.g. [`-DMETRICS_USER_DEFINES=\"src/my_metrics.def\"`](https://github.com/onkwon/libmcu/blob/master/project/runner.mk#L10)
3. Set metric values with APIs. e.g. `metrics_set(BatteryPCT, val)`
4. Set a timer to aggregate and send/save metrics periodically

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
[libcbor](https://github.com/libmcu/libcbor) dependency to use it.

To get the exact encoded payload size before allocation, call
`metrics_collect(NULL, 0)`.

### Reporting

`metrics_report()` drives a single collect-transmit cycle. Override the
following hooks to integrate with your transport and application logic:

| Hook | Role | Default |
| ---- | ---- | ------- |
| `metrics_report_prepare(ctx)` | Refresh metric values before collection (e.g. uptime, CPU load) | no-op |
| `metrics_report_transmit(data, size, ctx)` | Send the encoded payload over the desired transport (HTTP, MQTT, UART, …) | returns `-ENOSYS` |

Pass a `metricfs` instance to enable persistent storage: on transmit failure
the payload is saved to `metricfs` and retransmitted on the next cycle. Pass
`NULL` to disable persistence.

```c
uint8_t buf[256];
struct metricfs *mfs = /* initialised metricfs instance, or NULL */;

int err = metrics_report(buf, sizeof(buf), mfs, NULL);
/* 0 = ok, -EAGAIN = unsent data remains in store, <0 = error */
```

For periodic reporting, use the convenience wrapper
`metrics_report_periodic()`.  It calls `metrics_report()` only when the
configured interval has elapsed (default 60 minutes).  If unsent data remains
in `metricfs`, the interval is bypassed so the backlog drains immediately.

Return values:

| Value | Meaning |
| ----- | ------- |
| `0` | Report transmitted successfully |
| `-EALREADY` | Interval not yet elapsed and no backlog — skipped |
| `-EAGAIN` | Transmitted but unsent data still remains in store |
| other `< 0` | Error (e.g. `-EINVAL`, `-ENOBUFS`) |

The function is designed for repeated calls in a loop. When no backlog exists,
calls within the interval return `-EALREADY` immediately with no I/O. When a
backlog exists (e.g. after a transmit failure), the interval check is bypassed
and every call attempts to drain the stored data until the backlog is cleared.

```c
/* Override the default 60-minute interval at compile time if needed:
 *   -DMETRICS_REPORT_INTERVAL_SEC=300   (5 minutes) */

while (1) {
	int err;
	do {
		err = metrics_report_periodic(buf, sizeof(buf), mfs, NULL);
	} while (err != -EALREADY);
	sleep(10);
}
```

### Synchronisation

Implement `metrics_lock()` and `metrics_unlock()` in case of multi threaded
environment.

```c
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void metircs_lock(void) {
	pthread_mutex_lock(&lock);
}
void metircs_unlock(void) {
	pthread_mutex_unlock(&lock);
}
```
