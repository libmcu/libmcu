# CLI

## Overview

## Integration Guide

```c
static size_t cli_read(void *buf, size_t bufsize)
{
	return fread(buf, bufsize, 1, stdin);
}

static size_t cli_write(const void *data, size_t datasize)
{
	return fwrite(data, datasize, 1, stdout);
}

static const cli_io_t io = {
	.read = cli_read,
	.write = cli_write,
};

static cli_cmd_t cmds[] = {
	{
		.name = "help",
		.func = help_func,
		.desc = "your description",
	},
	{
		.name = "test",
		.func = test_func,
	},
};

cli_t cli;

cli_init(&cli, &io, cmds, sizeof(cmds) / sizeof(cmds[0]));
#if never_return_unless_exit_command_received
cli_run(&cli);
#else
while (1) {
	cli_step(&cli);
}
#endif
```

### Adding new command

cli_cmd_error_t help_func(int argc, const char *argv[], const void *env)
{
	...
	return CLI_CMD_SUCCESS;
}
