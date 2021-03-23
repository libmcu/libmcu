# Logging

## Overview
![logging class diagram](../../docs/images/logging.png)

On ARM Cortex-M cores, it uses 128 bytes stack at most with no dynamic
allocation at all. And a log size is 16 bytes excluding user messages.

An example for storage implementation can be found
[examples/memory_storage.c](../../examples/memory_storage.c) and a simple server-side
script [tools/scripts/translate_log.py](../../tools/scripts/translate_log.py).

Writing a log in a structured way would help you see and trace easier on the
server side, something like:

```c
info("#battery %d%%", battery");
info("rssi %d", rssi); // #wifi unit:dBm
error("#i2c timeout");
```

## Integration Guide
