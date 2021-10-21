#include "libmcu/cli.h"
#include "libmcu/cli_command.h"
#include <string.h>
#include "libmcu/assert.h"

#if !defined(CLI_COMMAND_MAXLEN)
#define CLI_COMMAND_MAXLEN			32
#endif
#if !defined(CLI_ARGS_MAXLEN)
#define CLI_ARGS_MAXLEN				8
#endif
#define CLI_PROMPT				"$ "
#define CLI_PROMPT_OK				""
#define CLI_PROMPT_ERROR			"ERROR\r\n"
#define CLI_PROMPT_NOT_FOUND			"command not found\r\n"
#define CLI_PROMPT_START_MESSAGE		\
	"\r\n\r\nType 'help' to get a list of available commands.\r\n"
#define CLI_PROMPT_EXIT_MESSAGE			"EXIT\r\n"

static void readline(char *buf, size_t bufsize, const cli_io_t *io)
{
	size_t index = 0;
	char ch;

	io->write(CLI_PROMPT, strlen(CLI_PROMPT));

	while (index < (bufsize - 1)) {
		if (io->read(&ch, 1) != 1) {
			continue;
		}
		if (ch == '\r' || ch == '\n') {
			break;
		}
		if (ch == '\t') {
			continue;
		}
		if (ch == '\b') {
			if (index > 0) {
				index--;
				io->write("\b \b", 3);
			}
			continue;
		}

		buf[index] = ch;
		index++;

		io->write(&ch, 1);
	}

	io->write("\r\n", 2);

	buf[index] = '\n';
	buf[index+1] = '\0';
}

static int parse(char *str, const char *argv[])
{
	int argc = 0;
	char *p = str;
	argv[0] = str;
	while ((p = strpbrk(p + 1, " \n")) != NULL) {
		argv[++argc] = p + 1;
		*p = '\0';
	}

	return argc;
}

static void report_result(const cli_io_t *io, cli_cmd_error_t err,
		const cli_cmd_t *cmd)
{
	switch (err) {
	case CLI_CMD_SUCCESS:
		io->write(CLI_PROMPT_OK, strlen(CLI_PROMPT_OK));
		break;
	case CLI_CMD_INVALID_PARAM:
		if (cmd->desc) {
			io->write(cmd->desc, strlen(cmd->desc));
			io->write("\r\n", 2);
		}
		break;
	case CLI_CMD_NOT_FOUND:
		io->write(CLI_PROMPT_NOT_FOUND, strlen(CLI_PROMPT_NOT_FOUND));
		break;
	case CLI_CMD_ERROR:
		io->write(CLI_PROMPT_ERROR, strlen(CLI_PROMPT_ERROR));
		break;
	default:
		break;
	}
}

static cli_cmd_error_t process(int argc, const char *argv[],
		const cli_io_t *io)
{
	if (argc <= 0) {
		return CLI_CMD_BLANK;
	}

	cli_cmd_error_t rc = CLI_CMD_NOT_FOUND;
	const cli_cmd_t *commands = cli_get_command_list();
	const cli_cmd_t *cmd;

	for (int i = 0; (cmd = &commands[i]) && cmd->name; i++) {
		if (strcmp(cmd->name, argv[0]) == 0) {
			rc = cmd->run(argc, argv, io);
			break;
		}
	}

	report_result(io, rc, cmd);

	return rc;
}

void cli_run(const cli_io_t *io)
{
	assert(io != NULL);

	char readbuf[CLI_COMMAND_MAXLEN + 1];
	cli_cmd_error_t rc;

	int argc;
	const char *argv[CLI_ARGS_MAXLEN];

	io->write(CLI_PROMPT_START_MESSAGE, strlen(CLI_PROMPT_START_MESSAGE));

	do {
		readline(readbuf, sizeof(readbuf), io);
		argc = parse(readbuf, argv);
		rc = process(argc, argv, io);
	} while (rc != CLI_CMD_EXIT);

	io->write(CLI_PROMPT_EXIT_MESSAGE, strlen(CLI_PROMPT_EXIT_MESSAGE));
}
