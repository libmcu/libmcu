# retry

## Overview

## Integration Guide

`retry_generate_random()` and `retry_sleep_ms()` should be implemented first.

```c
struct retry_params retry = { 0, };
do {
	if (do_something() == true) {
		break;
	}
} while (retry_backoff(&retry) != RETRY_EXHAUSTED);
```

The default can be set by defining `RETRY_DEFAULT_MAX_BACKOFF_MS`,
`RETRY_DEFAULT_MIN_BACKOFF_MS`, and `RETRY_DEFAULT_MAX_JITTER_MS`.
