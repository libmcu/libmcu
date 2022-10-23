# Trace

## Integration Guide
The compiler flag `-finstrument-functions` should be passed to activate tracing
functionality.

Add the following line to your CMake file:
`target_compile_options(${your-target} PRIVATE -finstrument-functions)`

or `CFLAGS += -finstrument-functions` in case of Makefile.

The default number of tracing slots is 128 which can hold up to 128 function
calls. The maximum number of slots can be changed by `TRACE_MAXLEN` define.

### Additional Information
To get timestamp, `uint32_t trace_get_time(void)` should be implemented.
Otherwise timestamp will always be 0.

To get stack usage, `size_t trace_stack_watermark(void)` should be implemented.
Otherwise stack usage will always be 0.

To get the TID, `void *trace_get_current_thread(void)` should be implemented.
Otherwise TID will always be `NULL`.

`LIBMCU_NO_INSTRUMENT` should be put to prevent infinite recursion when you
implement your own ones:

```c
LIBMCU_NO_INSTRUMENT
uint32_t trace_get_time(void) {
	return 	(uint32_t)xTaskGetTickCount();
}
```

### Hooks
The below functions will be called every function calls. Those functions can be
overridden by creating another functions with the same declaration.

* `void trace_enter_hook(const struct trace *entry)`
* `void trace_leave_hook(const struct trace *entry)`
