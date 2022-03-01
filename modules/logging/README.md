# Logging

## Overview
![logging class diagram](../../docs/images/logging.png)

On ARM Cortex-M cores, it uses 56 bytes stack and 181 bytes static memory at
most with no dynamic allocation at all. And a log size is 17 bytes excluding
user messages.

An example for storage implementation can be found
[examples/memory_storage.c](../../examples/memory_storage.c) and a simple server-side
script [tools/scripts/translate_log.py](../../tools/scripts/translate_log.py).

## Integration Guide

* `LOGGING_MESSAGE_MAXLEN` : The default is 80 bytes
* `LOGGING_TAGS_MAXNUM` : The default is 8
  - The unregistered tags over `LOGGING_TAGS_MAXNUM` share the global tag information
* `LOGGING_TAG` : The default is `__FILE__`
  - The shorter `__FILE__` the more code size preserved when you use the default
* `get_program_counter()`

### Custom TAG instead of __FILE__
Please refer to [test case](https://github.com/onkwon/libmcu/blob/master/tests/src/logging/logging_test.cpp#L10) and [makefile](https://github.com/onkwon/libmcu/blob/master/tests/runners/logging/logging.mk#L15).

### Syncronization

Implement `logging_lock_init()`, `logging_lock()` and `logging_unlock()` in case
of multi threaded environment.

## Example

```c
uint8_t logbuf[512];
logging_init(memory_storage_init(logbuf, sizeof(logbuf)));

debug("message");
logging_set_level_current(LOGGING_TYPE_INFO);
info("RSSI %d", rssi);
logging_set_level(TAG, LOGGING_TYPE_ERROR);
error("i2c timeout");
```
