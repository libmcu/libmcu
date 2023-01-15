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

enum cli_special_key {
	CTRL_B = 0x02, /* left */
	CTRL_C = 0x03,
	CTRL_F = 0x06, /* right */
	TAB    = 0x09,
	CTRL_N = 0x0E, /* down */
	CTRL_P = 0x10, /* up */
	ESC    = 0x1B,
};

enum cli_escape_char {
	UP    = 0x5B41, /* [A */
	DOWN  = 0x5B42, /* [B */
	RIGHT = 0x5B43, /* [C */
	LEFT  = 0x5B44, /* [D */
};

static uint16_t history_cap(const struct cli *cli)
{
	/* Index 0 buffer is used for scratch */
	return (uint16_t)(cli->bufsize / CLI_CMD_MAXLEN - 1);
}

static char *get_history_buffer(const struct cli *cli, uint16_t index)
{
	if (history_cap(cli) == 0) {
		return cli->buf;
	}

	return &cli->buf[(index + 1) * CLI_CMD_MAXLEN];
}

static uint16_t get_active_history_index(const struct cli *cli, int distance)
{
	uint16_t cap = history_cap(cli);
	if (cap == 0) {
		return 0;
	}
	return (uint16_t)(cap + distance + cli->history_active) % cap;
}

static void update_active_history(struct cli *cli, int distance)
{
	uint16_t index = get_active_history_index(cli, distance);
	cli->history_active = index;
}

static char *get_history(const struct cli *cli, int distance)
{
	uint16_t index = get_active_history_index(cli, distance);
	return get_history_buffer(cli, index);
}

static uint16_t get_history_and_update_active(struct cli *cli,
		char *buf, int distance)
{
	uint16_t index = get_active_history_index(cli, distance);
	uint16_t len = 0;

	const char *p = get_history_buffer(cli, index);
	len = (uint16_t)strlen(p);

	if (len) {
		memcpy(buf, p, len);

		cli->history_active = index;
	}

	return len;
}

static void set_history(struct cli *cli, const char *line)
{
	cli->history_active = cli->history_next;

	/* 1. skip if same to the previous history */
	if (strlen(line) == 0 || strcmp(get_history(cli, -1), line) == 0) {
		return;
	}

	/* 2. copy it to history buffer */
	char *p = get_history_buffer(cli, cli->history_next);
	strncpy(p, line, CLI_CMD_MAXLEN-1);

	/* 3. increse history next index */
	if (history_cap(cli)) {
		cli->history_next = (uint16_t)
			((cli->history_next + 1) % history_cap(cli));
		cli->history_active = cli->history_next;
	}
}

static void delete_line(const struct cli *cli, int n)
{
	for (int i = 0; i < n; i++) {
		cli->io->write("\b", 1);
	}
	for (int i = 0; i < n; i++) {
		cli->io->write(" ", 1);
	}
	for (int i = 0; i < n; i++) {
		cli->io->write("\b", 1);
	}
}

static void replace_line_with_history(struct cli *cli,
		char *buf, uint16_t *pos, int history)
{
	uint16_t len = get_history_and_update_active(cli, buf, history);

	if (len) {
		delete_line(cli, *pos);
		cli->io->write(buf, len);
		*pos = len;
	} else if (history > 0 && *pos) { /* scratch buffer for new command */
		delete_line(cli, *pos);
		update_active_history(cli, 1);
		*pos = 0;
	}
}

static void process_escape(struct cli *cli, char *buf, uint16_t *pos)
{
	uint8_t in[2];

	if (cli->io->read(&in[0], 1) != 1 ||
			cli->io->read(&in[1], 1) != 1) {
		return;
	}

	uint16_t key = (uint16_t)((in[0] << 8) | in[1]);

	switch (key) {
	case UP:
		replace_line_with_history(cli, buf, pos, -1);
		break;
	case DOWN:
		replace_line_with_history(cli, buf, pos, 1);
		break;
	default:
		break;
	}
}

static char *readline(struct cli *cli)
{
	char *prev = &cli->previous_input;
	char *res = NULL;
	char ch;

	if (cli->io->read(&ch, 1) != 1) {
		return NULL;
	}

	char *buf = cli->buf;
	uint16_t *pos = &cli->cursor_pos;

	switch (ch) {
	case CTRL_C:
		*pos = 0;
		/* fall through */
	case '\n': /* fall through */
	case '\r': /* carriage return */
		if (ch != *prev && (*prev == '\n' || *prev == '\r')) {
			ch = 0; /* to deal with consecutive empty lines */
			break;
		}

		buf[(*pos)++] = '\0';
		*pos = 0;

		cli->io->write("\n", 1);
		res = buf;
		break;
	case '\b': /* backspace */
		if (*pos > 0) {
			(*pos)--;
			cli->io->write("\b \b", 3);
		}
		break;
	case '\t': /* tab */
		break;
	case CTRL_P:
		replace_line_with_history(cli, buf, pos, -1);
		break;
	case CTRL_N:
		replace_line_with_history(cli, buf, pos, 1);
		break;
	case ESC:
		process_escape(cli, buf, pos);
		break;
	default:
		if (*pos >= CLI_CMD_MAXLEN) {
			break;
		}

		buf[(*pos)++] = ch;
		cli->io->write(&ch, 1);
		break;
	}

	*prev = ch;
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

		if (str[i+1] == '\0') {
			break;
		}

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

	set_history(cli, line);

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
	cli->history_next = 0;
	cli->history_active = 0;
	cli->buf = (char *)buf;
	cli->bufsize = bufsize;

	memset(buf, 0, bufsize);

	io->write(CLI_PROMPT_START_MESSAGE, strlen(CLI_PROMPT_START_MESSAGE));
	io->write(CLI_PROMPT, strlen(CLI_PROMPT));
}
