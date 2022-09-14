/*
 * SPDX-FileCopyrightText: 2021 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "cli_commands.h"
#include <stddef.h>

const cli_cmd_t cli_commands[] = {
	{"exit", cli_cmd_exit, "Exit the CLI" },
	{"help", cli_cmd_help, "List available commands" },
	{"info", cli_cmd_info, "info [version|sn|build] to get device info" },
	{"reboot", cli_cmd_reboot, "Reboot the device" },
	{"md", cli_cmd_memdump, "md <addr> <len>" },
};
