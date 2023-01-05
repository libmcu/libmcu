/*
 * SPDX-FileCopyrightText: 2021 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/cli.h"
#include "libmcu/compiler.h"
#include "libmcu/board.h"

DEFINE_CLI_CMD(reboot, "Reboot the device") {
	unused(argc);
	unused(argv);
	unused(env);

	board_reboot();

	return CLI_CMD_SUCCESS;
}
