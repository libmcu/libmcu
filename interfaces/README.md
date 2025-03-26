# Interface Design Guide

The `interfaces/` layer is designed to provide a minimal, stable, and portable
API surface that clearly separates high-level application logic from low-level,
platform-specific drivers. These conventions exist to ensure:

- Safe integration with different platforms
- Maintainability across modules
- Testability, determinism, and clarity

## API Design Philosophy

- Prefix `lm_` is used consistently for all global symbols to avoid name collisions.
- Static functions and internal headers are used to separate implementation details.
- Dynamic memory allocation is avoided in all runtime paths; however, controlled, non-fragmenting allocation may be allowed during initialization.
- Each module (UART, GPIO, etc.) lives in its own directory with clean separation.
- RTOS-aware: designed to integrate with FreeRTOS, Zephyr, or any custom RTOS via backend abstraction.
- Synchronous-by-default API: Async variants always carry `_async` suffix.

> Async APIs typically follow an event-driven or callback-based model, where an operation is initiated and control returns immediately. Completion is signaled via callbacks, IRQs, or message queues.

### Instance Model

_libmcu_ adopts a **multi-instance architecture** for peripherals that commonly
support multiple hardware channels, such as UART, SPI, or I2C. Instead of
relying on global singleton state, APIs return an opaque handle that
encapsulates instance-specific state:

```c
struct lm_uart *uart = lm_uart_create(1, &config);
lm_uart_write(uart, data, len);
lm_uart_destroy(uart);
```

This approach improves scalability, reentrancy, and testability. It also
enables clean integration in unit tests and avoids global state pollution.
Singleton-style APIs may still be used for system-level resources where
instance management is unnecessary.

## Style Guide

### Naming Conventions

- All public symbols use `lm_` prefix.
- Structures are declared using `struct` syntax, not `typedef`.
- Avoid `_t` suffixes for user-defined types — reserved by POSIX for system types.
- Prefer `config` or `cfg` as local variable names depending on context and clarity.

### API Rules

- Synchronous (blocking) APIs are the default.
- Asynchronous APIs must include `_async` suffix.
  - For example, `lm_uart_write()` is blocking by default, while `lm_uart_write_async()` enables non-blocking usage with a callback.
- Avoid dynamic memory allocation in runtime paths; controlled initialization-time usage is allowed if fragmentation is avoided.
- All I/O functions must be safe to call from interrupt context unless explicitly documented otherwise.

#### Resource Lifecycle Semantics

- Use `create()` / `destroy()` for APIs that allocate and free memory explicitly or via pool.
- Use `open()` / `close()` when accessing statically defined or shared system resources.
- `create()` implies the caller **owns** the resource and is responsible for its release.
- `open()` implies the caller **accesses** an existing shared resource.
- Always match the semantic meaning with the resource ownership model.
