# Shell

## Overview

## Integration Guide

```c
static size_t shell_read(void *buf, size_t bufsize)
{
	return fread(buf, bufsize, 1, stdin);
}

static size_t shell_write(const void *data, size_t datasize)
{
	return fwrite(data, datasize, 1, stdout);
}

const shell_io_t io = {
	.read = shell_read,
	.write = shell_write,
};

shell_run(&io);
```

### Adding new command
