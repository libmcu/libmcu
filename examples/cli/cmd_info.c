/*
 * SPDX-FileCopyrightText: 2021 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/cli.h"
#include <string.h>
#include "libmcu/board.h"

typedef enum {
	CMD_OPT_NONE			= 0x00,
	CMD_OPT_VERSION			= 0x01,
	CMD_OPT_BUILD_DATE		= 0x02,
	CMD_OPT_SERIAL_NUMBER		= 0x04,
	CMD_OPT_HELP			= 0x08,
	CMD_OPT_ALL			= (
			CMD_OPT_VERSION |
			CMD_OPT_BUILD_DATE |
			CMD_OPT_SERIAL_NUMBER),
} cmd_opt_t;

static void println(const struct cli_io *io, const char *str)
{
	io->write(str, strlen(str));
	io->write("\n", 1);
}

static cmd_opt_t get_command_option(int argc, const char *opt)
{
	if (argc == 1) {
		return CMD_OPT_ALL;
	} else if (opt == NULL) {
		return CMD_OPT_NONE;
	} else if (strcmp(opt, "help") == 0) {
		return CMD_OPT_HELP;
	} else if (strcmp(opt, "version") == 0) {
		return CMD_OPT_VERSION;
	} else if (strcmp(opt, "sn") == 0) {
		return CMD_OPT_SERIAL_NUMBER;
	} else if (strcmp(opt, "build") == 0) {
		return CMD_OPT_BUILD_DATE;
	}

	return CMD_OPT_ALL;
}

static void print_help(struct cli_io const *io)
{
	println(io, "subcommands:\n\n\tversion\n\tsn\n\tbuild");
}

static void print_version(struct cli_io const *io)
{
	println(io, board_get_version_string());
}

static void print_sn(struct cli_io const *io)
{
	println(io, board_get_serial_number_string());
}

static void print_build_date(struct cli_io const *io)
{
	println(io, board_get_build_date_string());
}

DEFINE_CLI_CMD(info, "Display device info") {
	if (argc > 2) {
		return CLI_CMD_INVALID_PARAM;
	}

	struct cli const *cli = (struct cli const *)env;

	cmd_opt_t options = get_command_option(argc, argv? argv[1] : NULL);

	if (options & CMD_OPT_HELP) {
		print_help(cli->io);
	}
	if (options & CMD_OPT_VERSION) {
		print_version(cli->io);
	}
	if (options & CMD_OPT_SERIAL_NUMBER) {
		print_sn(cli->io);
	}
	if (options & CMD_OPT_BUILD_DATE) {
		print_build_date(cli->io);
	}

	return CLI_CMD_SUCCESS;
}
