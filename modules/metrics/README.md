# Metrics

## Integration Guide

1. Define your metrics with the `METRICS_DEFINE` macro in a file. e.g. [metrics.def](metrics.def)
2. Let the compiler know where your file is located using `METRICS_USER_DEFINES`
   macro[^1]. e.g. [`-DMETRICS_USER_DEFINES=\"src/my_metrics.def\"`](https://github.com/onkwon/libmcu/blob/master/config/runner.mk#L10)
3. Set metric values with APIs. e.g. `metrics_set(BatteryPCT, val)`
4. Set a timer to aggregate and send/save metrics periodically

[^1]: The default file name is `metrics.def`. You don't need to specify the file
location with `METRICS_USER_DEFINES` when you use the default file name and the
file is in the include path.

### Encoding

You can implement your own encoder using `metrics_encode_header()` and
`metrics_encode_each()`. No encoder by default, meaning just a simple byte
stream.

### Syncronization

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
