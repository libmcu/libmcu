# ratelim
ratelim is a lightweight rate-limiting module based on the Leaky Bucket Algorithm. It is designed to prevent system overload by limiting request or operation rates while maintaining steady data flow.

## Key Features
- Leaky Bucket Algorithm: Smoothly processes requests at a consistent rate.
- Burst Prevention: Avoids system overload by limiting request bursts.
- Dynamic Configuration: Flexible initialization with customizable capacity and leak rate.
- Convenience Functionality: Supports immediate request handling via callback invocation.

## How It Works
The module implements a virtual "bucket":

1. Requests are added to the bucket.
2. The bucket "leaks" requests at a fixed rate, simulating steady processing.
3. If the bucket is full, additional requests are denied or deferred.

## Usage
### Initialization
Initialize a rate limiter with a defined capacity and leak rate:

```c
#include "libmcu/ratelim.h"

struct ratelim bucket;
ratelim_init(&bucket, RATELIM_UNIT_SECONCD, 10, 2); // Capacity: 10, Leak rate: 2 requests/second
```

### Request Handling
#### Adding a Request
To add a request to the bucket and check if it is allowed:

```c
if (ratelim_request(&bucket)) {
    // Request allowed, process it
} else {
    // Request denied
}
```

### Formatted Request Handling
Function Prototype

```c
bool ratelim_request_format(struct ratelim *bucket,
        ratelim_format_func_t func, const char *format, ...);
```

- bucket: Pointer to the rate limiter structure.
- func: A variadic function pointer (e.g., printf or a custom logger).
- format: Format string for the variadic function.
- ...: Variadic arguments corresponding to the format string.

#### Example: Rate-Limited printf

```c
ratelim_request_format(&bucket, printf, "Hello, world! %d %s\n", 42, "example");
```

#### Example: Custom Logger

```c
#include <stdio.h>

// Custom variadic logging function
void custom_logger(const char *format, va_list args) {
    printf("[LOG]: ");
    vprintf(format, args);
}

// Usage
ratelim_request_format(&bucket, custom_logger, "System status: %s, Error code: %d\n", "OK", 0);
```

## Advantages
- Modularity: Provides flexible APIs for direct request handling or callback-based execution.
- Efficiency: Lightweight implementation with minimal resource overhead.
- Burst Protection: Ensures steady request handling without overloading the system.

## Use Cases
- API Rate Limiting: Prevent excessive API calls within a short period.
- Task Scheduling: Limit periodic task execution rates.
- Traffic Shaping: Control data flow in networking applications.
