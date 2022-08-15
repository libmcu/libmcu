# CLI

## Overview

## Integration Guide

```c
static size_t cli_read(void *buf, size_t bufsize)
{
	return fread(buf, bufsize, 1, stdin);
}

static size_t cli_write(void const *data, size_t datasize)
{
	return fwrite(data, datasize, 1, stdout);
}

static const cli_io_t io = {
	.read = cli_read,
	.write = cli_write,
};

static struct cli_cmd cmds[] = {
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

struct cli cli;

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

### Increasing the command buffer size

`-DCLI_CMD_MAXLEN=128`

### Increasing the maximum command arguments

`-DCLI_CMD_ARGS_MAXLEN=8`

### Customizing the messages

- `-DCLI_PROMPT=""`
- `-DCLI_PROMPT_OK=""`
- `-DCLI_PROMPT_ERROR=""`
- `-DCLI_PROMPT_NOT_FOUND=""`
- `-DCLI_PROMPT_START_MESSAGE=""`
- `-DCLI_PROMPT_EXIT_MESSAGE=""`
