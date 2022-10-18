# Logging

## Overview
On ARM Cortex-M cores, it uses 56 bytes stack and 181 bytes static memory at
most with no dynamic allocation at all. And a log size is 17 bytes excluding
a user custom message.

An example for a backend implementation can be found
[examples/memory_storage.c](../../examples/memory_storage.c) and a simple server-side
script [tools/scripts/translate_log.py](../../tools/scripts/translate_log.py).

## Integration Guide

* `LOGGING_MESSAGE_MAXLEN` : The default is 80 bytes
* `LOGGING_TAGS_MAXNUM` : The default is 8
  - The unregistered tags over `LOGGING_TAGS_MAXNUM` share the global tag information
* `LOGGING_TAG` : The default is `__FILE__`
  - The shorter `__FILE__` the more code size preserved when you use the default
* `get_program_counter()`
* `LOGGING_MAX_BACKENDS` : The default is 1

### Custom TAG instead of `__FILE__`
Please refer to [test case](https://github.com/onkwon/libmcu/blob/master/tests/src/logging/logging_test.cpp#L10) and [makefile](https://github.com/onkwon/libmcu/blob/master/tests/runners/logging/logging.mk#L15).

### Syncronization

Implement `logging_lock_init()`, `logging_lock()` and `logging_unlock()` in case
of multi threaded environment.

## Example

```c
static uint8_t logbuf[512];

logging_init();
logging_add_backend(memory_storage_init(logbuf, sizeof(logbuf)));
logging_set_level(TAG, LOGGING_TYPE_DEBUG);

debug("message");
info("RSSI %d", rssi);
error("i2c timeout");
```
