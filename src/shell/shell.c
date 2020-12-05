#include "libmcu/shell.h"
#include <string.h>
#include "shell_command.h"
#include "commands/commands.h"

#if !defined(SHELL_COMMAND_MAXLEN)
#define SHELL_COMMAND_MAXLEN			32
#endif
#if !defined(SHELL_ARGS_MAXLEN)
#define SHELL_ARGS_MAXLEN			8
#endif
#define SHELL_PROMPT				"$ "
#define SHELL_PROMPT_OK				""
#define SHELL_PROMPT_ERROR			"ERROR\r\n"
#define SHELL_PROMPT_EXIT			"EXIT\r\n"
#define SHELL_PROMPT_NOT_FOUND			"command not found\r\n"

static void readline(char *buf, size_t bufsize, const shell_io_t *io)
{
	size_t index = 0;
	char ch;

	io->write(SHELL_PROMPT, strlen(SHELL_PROMPT));

	while (index < (bufsize - 1)) {
		if (io->read(&ch, 1) != 1) {
			continue;
		}
		if (ch == '\r' || ch == '\n') {
			break;
		}

		io->write(&ch, 1);

		if (index && ch == '\b') {
			index--;
			continue;
		}

		buf[index] = ch;
		index++;
	}

	ch = '\r';
	io->write(&ch, 1);
	ch = '\n';
	io->write(&ch, 1);

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

static shell_cmd_error_t process(int argc, const char *argv[],
		const shell_io_t *io)
{
	if (argc <= 0) {
		return SHELL_CMD_BLANK;
	}

	shell_cmd_error_t rc = SHELL_CMD_NOT_FOUND;
	const shell_cmd_t **commands = shell_get_command_list();
	const shell_cmd_t *cmd;

	for (int i = 0; (cmd = commands[i]) != NULL; i++) {
		if (strcmp(cmd->name, argv[0]) == 0) {
			rc = cmd->run(argc, argv, io);
			break;
		}
	}

	switch (rc) {
	case SHELL_CMD_SUCCESS:
		io->write(SHELL_PROMPT_OK, strlen(SHELL_PROMPT_OK));
		break;
	case SHELL_CMD_INVALID_PARAM:
		io->write(cmd->desc, strlen(cmd->desc));
		break;
	case SHELL_CMD_NOT_FOUND:
		io->write(SHELL_PROMPT_NOT_FOUND, strlen(SHELL_PROMPT_NOT_FOUND));
		break;
	case SHELL_CMD_EXIT:
		io->write(SHELL_PROMPT_EXIT, strlen(SHELL_PROMPT_EXIT));
		break;
	case SHELL_CMD_ERROR:
		io->write(SHELL_PROMPT_ERROR, strlen(SHELL_PROMPT_ERROR));
		break;
	default:
	case SHELL_CMD_BLANK: // for compiler to not complain
		break;
	}

	return rc;
}

void shell_run(const shell_io_t *io)
{
	char readbuf[SHELL_COMMAND_MAXLEN + 1];
	shell_cmd_error_t rc;

	int argc;
	const char *argv[SHELL_ARGS_MAXLEN];

	do {
		readline(readbuf, sizeof(readbuf), io);
		argc = parse(readbuf, argv);
		rc = process(argc, argv, io);
	} while (rc != SHELL_CMD_EXIT);
}
