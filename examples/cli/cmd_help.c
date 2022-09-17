/*
 * SPDX-FileCopyrightText: 2021 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "cli_commands.h"
#include <string.h>
#include "libmcu/cli.h"
#include "libmcu/compiler.h"

cli_cmd_error_t cli_cmd_help(int argc, const char *argv[], const void *env)
{
	unused(argc);
	unused(argv);

	struct cli const *cli = (struct cli const *)env;

	for (size_t i = 0; i < cli->cmdlist_len; i++) {
		struct cli_cmd const *cmd = &cli->cmdlist[i];
		cli->io->write(cmd->name, strlen(cmd->name));
		if (cmd->desc) {
			cli->io->write("\t: ", 3);
			cli->io->write(cmd->desc, strlen(cmd->desc));
		}
		cli->io->write("\r\n", 2);
	}

	return CLI_CMD_SUCCESS;
}
