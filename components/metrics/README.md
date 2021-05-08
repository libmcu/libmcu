# Metrics

## Integration Guide

1. Define your metrics with the `METRICS_DEFINE` macro in a file. e.g. [metrics.def](include/metrics.def)
2. Let the compiler know where your file is located using `METRICS_USER_DEFINES`
   macro[^1]. e.g. [`-DMETRICS_USER_DEFINES=\"src/my_metyrics.def\"`](https://github.com/onkwon/libmcu/blob/master/projects/runner.mk#L10)
3. Set metric values with APIs. e.g. `metrics_set(BatteryPCT, val)`
4. Set a timer to aggregate and send/save metrics periodically

[^1]: The default file name is `metrics.def`. You don't need to specify the file
location with `METRICS_USER_DEFINES` when you use the default file name and the
file is in the include path.
