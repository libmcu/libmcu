/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/cli.h"
#include <stdint.h>
#include <string.h>

#if !defined(CLI_PROMPT)
#define CLI_PROMPT				"$ "
#endif
#if !defined(CLI_PROMPT_OK)
#define CLI_PROMPT_OK				""
#endif
#if !defined(CLI_PROMPT_ERROR)
#define CLI_PROMPT_ERROR			"ERROR\n"
#endif
#if !defined(CLI_PROMPT_NOT_FOUND)
#define CLI_PROMPT_NOT_FOUND			"command not found\n"
#endif
#if !defined(CLI_PROMPT_START_MESSAGE)
#define CLI_PROMPT_START_MESSAGE		\
	"\n\nType 'help' to get a list of available commands.\n"
#endif
#if !defined(CLI_PROMPT_EXIT_MESSAGE)
#define CLI_PROMPT_EXIT_MESSAGE			"EXIT\n"
#endif

static char *readline(struct cli *cli)
{
	static char prev;
	char *res = NULL;
	char ch;

	if (cli->io->read(&ch, 1) != 1) {
		return NULL;
	}

	char *buf = cli->buf;
	uint16_t *pos = &cli->cursor_pos;

	switch (ch) {
	case '\n': /* fall through */
	case '\r': /* carriage return */
		if (ch != prev && (prev == '\n' || prev == '\r')) {
			ch = 0; /* to deal with consecutive empty lines */
			break;
		}

		buf[(*pos)++] = '\0';
		*pos = 0;

		cli->io->write("\n", 1);
		res = buf;
		break;
	case '\b': /* back space */
		if (*pos > 0) {
			(*pos)--;
			cli->io->write("\b \b", 3);
		}
		break;
	case '\t': /* tab */
		break;
		break;
	default:
		if (*pos >= CLI_CMD_MAXLEN) {
			break;
		}

		buf[(*pos)++] = ch;
		cli->io->write(&ch, 1);
		break;
	}

	prev = ch;
	return res;
}

static bool is_delimeter(char ch, char *delimeter)
{
	bool rc = false;

	if (ch == '\"') {
		*delimeter = *delimeter == '\"'? ' ' : '\"';
		rc = true;
		goto out;
	}

	if (*delimeter == '\"') {
		goto out;
	}

	if (ch == ' ' || ch == '\n') {
		rc = true;
	}

out:
	return rc;
}

static int count_leading_spaces(char const *str)
{
	for (int i = 0; str[i] != '\0'; i++) {
		if (str[i] != ' ') {
			return i;
		}
	}

	return 0;
}

static int parse_command(char *str, char const *argv[], size_t maxargs)
{
	int argc = 0;
	int i = count_leading_spaces(str);
	argv[0] = &str[i];

	for (char delimeter = ' '; str[i] != '\0'; i++) {
		if (!is_delimeter(str[i], &delimeter)) {
			continue;
		}

		str[i] = '\0';
		i += count_leading_spaces(&str[i+1]);

		if ((uintptr_t)argv[argc] != (uintptr_t)(str + i) &&
				(size_t)argc < maxargs) {
			argc += 1;
		}

		argv[argc] = str + i + 1;
	}

	return argc + !!i;
}

static void report_result(struct cli_io const *io, cli_cmd_error_t err,
		struct cli_cmd const *cmd)
{
	switch (err) {
	case CLI_CMD_SUCCESS:
		io->write(CLI_PROMPT_OK, strlen(CLI_PROMPT_OK));
		break;
	case CLI_CMD_INVALID_PARAM:
		if (cmd && cmd->desc) {
			io->write(cmd->desc, strlen(cmd->desc));
			io->write("\n", 1);
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

static cli_cmd_error_t process_command(struct cli const *cli,
		int argc, char const *argv[], void const *env)
{
	if (argc <= 0) {
		return CLI_CMD_BLANK;
	}

	cli_cmd_error_t rc = CLI_CMD_NOT_FOUND;
	struct cli_cmd const *cmd = NULL;

	for (size_t i = 0; cli->cmdlist && cli->cmdlist[i]; i++) {
		cmd = cli->cmdlist[i];
		if (strcmp(cmd->name, argv[0]) == 0) {
			rc = cmd->func(argc, argv, env);
			break;
		}
	}

	report_result(cli->io, rc, cmd);

	return rc;
}

static cli_cmd_error_t cli_step_core(struct cli *cli)
{
	char *line;

	if (!(line = readline(cli))) {
		return CLI_CMD_SUCCESS;
	}

	char const *argv[CLI_CMD_ARGS_MAXLEN];
	int argc = parse_command(line, argv, CLI_CMD_ARGS_MAXLEN);

	cli_cmd_error_t err = process_command(cli, argc, argv, cli);

	if (err != CLI_CMD_EXIT) {
		cli->io->write(CLI_PROMPT, strlen(CLI_PROMPT));
	}

	return err;
}

void cli_step(struct cli *cli)
{
	cli_step_core(cli);
}

void cli_run(struct cli *cli)
{
	cli_cmd_error_t rc;

	do {
		rc = cli_step_core(cli);
	} while (rc != CLI_CMD_EXIT);

	cli->io->write(CLI_PROMPT_EXIT_MESSAGE,
			strlen(CLI_PROMPT_EXIT_MESSAGE));
}

void cli_register_cmdlist(struct cli *cli, const struct cli_cmd **cmdlist)
{
	CLI_ASSERT(cli != NULL && cmdlist != NULL);

	cli->cmdlist = cmdlist;
}

void cli_init(struct cli *cli, struct cli_io const *io,
		void *buf, size_t bufsize)
{
	CLI_ASSERT(cli != NULL && io != NULL &&
			buf != NULL && bufsize >= CLI_CMD_MAXLEN);

	cli->io = io;
	cli->cmdlist = NULL;
	cli->cursor_pos = 0;
	cli->buf = buf;
	cli->bufsize = bufsize;

	io->write(CLI_PROMPT_START_MESSAGE, strlen(CLI_PROMPT_START_MESSAGE));
	io->write(CLI_PROMPT, strlen(CLI_PROMPT));
}
