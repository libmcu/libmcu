/*
 * SPDX-FileCopyrightText: 2021 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/cli.h"
#include <string.h>
#include "libmcu/compiler.h"

DEFINE_CLI_CMD(help, "List available commands") {
	unused(argc);
	unused(argv);

	struct cli const *cli = (struct cli const *)env;

	for (size_t i = 0; cli->cmdlist[i]; i++) {
		struct cli_cmd const *cmd = cli->cmdlist[i];
		cli->io->write(cmd->name, strlen(cmd->name));
		if (cmd->desc) {
			cli->io->write("\t: ", 3);
			cli->io->write(cmd->desc, strlen(cmd->desc));
		}
		cli->io->write("\n", 1);
	}

	return CLI_CMD_SUCCESS;
}
