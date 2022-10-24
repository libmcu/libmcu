/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/cli.h"
#include <stdint.h>
#include <string.h>
#include "libmcu/assert.h"
#include "libmcu/compiler.h"

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

static bool readline(struct cli *cli)
{
	static char prev;
	char ch;
	bool rc = false;

	if (cli->io->read(&ch, 1) != 1) {
		return false;
	}

	switch (ch) {
	case '\n': /* fall through */
	case '\r': /* carriage return */
		if (ch != prev && (prev == '\n' || prev == '\r')) {
			ch = 0; /* to deal with consecutive empty lines */
			break;
		}

		cli->cmdbuf[cli->cmdbuf_index++] = '\n';
		cli->cmdbuf[cli->cmdbuf_index++] = '\0';
		cli->cmdbuf_index = 0;

		cli->io->write("\n", 1);
		rc = true;
		break;
	case '\b': /* back space */
		if (cli->cmdbuf_index > 0) {
			cli->cmdbuf_index--;
			cli->io->write("\b \b", 3);
		}
		break;
	case '\t': /* tab */
		break;
	default:
		if (cli->cmdbuf_index >= CLI_CMD_MAXLEN) {
			break;
		}

		cli->cmdbuf[cli->cmdbuf_index++] = ch;
		cli->io->write(&ch, 1);
		break;
	}

	prev = ch;
	return rc;
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

	return argc;
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

	for (size_t i = 0; i < cli->cmdlist_len; i++) {
		cmd = &cli->cmdlist[i];
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
	if (!readline(cli)) {
		return CLI_CMD_SUCCESS;
	}

	char const *argv[CLI_CMD_ARGS_MAXLEN];
	int argc = parse_command(cli->cmdbuf, argv, CLI_CMD_ARGS_MAXLEN);

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

void cli_init(struct cli *cli, struct cli_io const *io,
	      struct cli_cmd const *cmdlist, size_t cmdlist_len)
{
	assert(cli != NULL && io != NULL && cmdlist != NULL);

	cli->io = io;
	cli->cmdlist = cmdlist;
	cli->cmdlist_len = cmdlist_len;
	cli->cmdbuf_index = 0;

	io->write(CLI_PROMPT_START_MESSAGE, strlen(CLI_PROMPT_START_MESSAGE));
	io->write(CLI_PROMPT, strlen(CLI_PROMPT));
}

struct cli_io const * LIBMCU_WEAK cli_io_create(void)
{
	return NULL;
}
