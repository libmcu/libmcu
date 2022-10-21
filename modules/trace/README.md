# Trace

## Integration Guide
The compiler flag `-finstrument-functions` should be passed to activate tracing
functionality.

Add the following line to your CMake file:
`target_compile_options(${your-target} PRIVATE -finstrument-functions)`

or `CFLAGS += -finstrument-functions` in case of Makefile.

The default number of tracing slots is 128 which can hold up to 128 function
calls. The maximum number of slots can be changed by `TRACE_MAXLEN` define.
