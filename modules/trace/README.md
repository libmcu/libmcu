# Trace

## Integration Guide
The compiler flag `-finstrument-functions` should be passed to activate tracing
functionality.

Add the following line to your CMake file:
`target_compile_options(${your-target} PRIVATE -finstrument-functions)`

or `CFLAGS += -finstrument-functions` in case of Makefile.

The default buffer size for tracing is 1024 bytes which can hold up to about
113 function calls. The buffer size can be changed by `TRACE_BUFSIZE` define.
