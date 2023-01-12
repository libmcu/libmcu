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

static char cli_buffer[CLI_CMD_MAXLEN * 1/* the number of history to keep*/];
struct cli cli;

DEFINE_CLI_CMD_LIST(cli_commands, test_cmd, your_cmd);
cli_init(&cli, &io, cli_buffer, sizeof(cli_buffer));
cli_register_cmdlist(&cli, cli_commands);
#if never_return_unless_exit_command_received
cli_run(&cli);
#else
while (1) {
	cli_step(&cli);
}
#endif
```

### Adding new command

```c
DEFINE_CLI_CMD(your_cmd, "Description for your command") {
	...
	return CLI_CMD_SUCCESS;
}

DEFINE_CLI_CMD(test_cmd, "Description for test command") {
	...
	return CLI_CMD_SUCCESS;
}
```

### Increasing the command buffer size

It should count the null character.

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
