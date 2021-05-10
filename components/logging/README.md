# Logging

## Overview
![logging class diagram](../../docs/images/logging.png)

On ARM Cortex-M cores, it uses 128 bytes stack at most with no dynamic
allocation at all. And a log size is 16 bytes excluding user messages.

An example for storage implementation can be found
[examples/memory_storage.c](../../examples/memory_storage.c) and a simple server-side
script [tools/scripts/translate_log.py](../../tools/scripts/translate_log.py).

## Integration Guide

* `LOGGING_MESSAGE_MAXLEN` : The default is 80 bytes
* `LOGGING_TAGS_MAXNUM` : The default is 8
* `LOGGING_TAG` : The default is `__FILE__`
  - The shorter `__FILE__` the more code size preserved when you use the default
* `get_program_counter()`

### Custom TAG instead of `__FILE__`
Please refer to [`-DLOGGING_TAG=TAG`](tests/runners/logging/logging.mk) and
[`static const char *TAG = "CUSTOM_TAG";`](tests/src/logging/logging_test.cpp).

```c
debug("message");
logging_set_level_current(LOGGING_TYPE_INFO);
info("RSSI %d", rssi);
logging_set_level(TAG, LOGGING_TYPE_ERROR);
error("i2c timeout");
```
