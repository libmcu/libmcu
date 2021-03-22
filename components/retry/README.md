# retry

## Overview

## Integration Guide

```c
retry_t retry = { .sleep = sleep_ms, };
do {
	if (do_something() == true) {
		break;
	}
} while (retry_backoff(&retry) != RETRY_EXHAUSTED);
```
