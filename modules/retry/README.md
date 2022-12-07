# retry

## Overview

## Integration Guide

`retry_generate_random()` and `retry_sleep_ms()` should be implemented first:

```c
#include "retry_overrides.h"

int retry_generate_random(void) {
	return your_random_function();
}
void retry_sleep_ms(unsigned int msec) {
	your_sleep_function(msec);
}
```

## Usage

```c
struct retry retry;
retry_init(&retry, 5, 30000, 5000, 5000);
do {
	if (connect() == SUCCESS) {
		break;
	}
} while (retry_backoff(&retry) != RETRY_EXHAUSTED);
```

or

```c
struct retry retry;
retry_init(&retry, 5, 30000, 5000, 5000);
uint32_t next_backoff_ms = retry_backoff_next(&retry));
```
