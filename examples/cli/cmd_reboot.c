/*
 * SPDX-FileCopyrightText: 2021 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "cli_commands.h"
#include "libmcu/compiler.h"
#include "libmcu/board.h"

cli_cmd_error_t cli_cmd_reboot(int argc, const char *argv[], const void *env)
{
	unused(argc);
	unused(argv);
	unused(env);

	board_reboot();

	return CLI_CMD_SUCCESS;
}
