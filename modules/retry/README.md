# Retry

## Overview

The `retry` module provides a mechanism to handle retry logic with exponential backoff and jitter.

## Functions

### `retry_new_static`

Creates a new retry structure with static allocation.

```c
retry_error_t retry_new_static(struct retry *self, const struct retry_param *param);
```

- `self`: Pointer to the retry structure.
- `param`: Pointer to the retry parameters.

### `retry_new`

Creates a new retry structure with dynamic allocation.

```c
struct retry *retry_new(const struct retry_param *param);
```

- `param`: Pointer to the retry parameters.

### `retry_delete`

Deletes a dynamically allocated retry structure.

```c
void retry_delete(struct retry *self);
```

- `self`: Pointer to the retry structure.


### `retry_backoff`

Calculates the next backoff time and updates the retry structure.

```c
retry_error_t retry_backoff(struct retry *self, uint32_t *next_backoff_ms, const uint16_t random_jitter);
```

- `self`: Pointer to the retry structure.
- `next_backoff_ms`: Pointer to store the next backoff time in milliseconds.
- `random_jitter`: For jitter calculation.

### `retry_get_backoff`

Returns the previous backoff time.

```c
uint32_t retry_get_backoff(const struct retry *self);
```

- `self`: Pointer to the retry structure.

### `retry_reset`

Resets the retry structure.

```c
void retry_reset(struct retry *self);
```

- `self`: Pointer to the retry structure.

### `retry_exhausted`

Checks if the retry attempts are exhausted.

```c
bool retry_exhausted(const struct retry *self);
```

- `self`: Pointer to the retry structure.

### `retry_first`

Checks if it is the first retry attempt.

```c
bool retry_first(const struct retry *self);
```

- `self`: Pointer to the retry structure.

## Usage

### Initialization

Initialize the retry structure with `retry_new_static`:

```c
struct retry retry;
struct retry_param param = {
    .min_backoff_ms = 1000,
    .max_backoff_ms = 32000,
    .max_attempts = 5,
    .max_jitter_ms = 500
};
retry_new_static(&retry, &param);
```

### Backoff with Retry

Use `retry_backoff` to perform the backoff and retry logic:

```c
uint32_t next_backoff_ms;

while (!retry_exhausted(&retry)) {
    if (connect() == SUCCESS) {
        break;
    }

    retry_backoff(&retry, &next_backoff_ms, random_number());
    sleep(next_backoff_ms);
}
```

### Calculate Next Backoff

Use `retry_get_backoff` to get the previous backoff time(or the current backoff time when called afterwards):

```c
uint32_t previous_backoff_ms = retry_get_backoff(&retry);
```

### Reset Retry

Use `retry_reset` to reset the retry structure:

```c
retry_reset(&retry);
```

### Check Retry Status

Use `retry_exhausted` to check if the retry attempts are exhausted:

```c
bool exhausted = retry_exhausted(&retry);
```

Use `retry_first` to check if it is the first retry attempt:

```c
bool first_attempt = retry_first(&retry);
```
