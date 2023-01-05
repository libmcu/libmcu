/*
 * SPDX-FileCopyrightText: 2021 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/cli.h"
#include "libmcu/compiler.h"

DEFINE_CLI_CMD(exit, "Exit the CLI") {
	unused(argc);
	unused(argv);
	unused(env);

	return CLI_CMD_EXIT;
}
