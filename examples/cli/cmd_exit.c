/*
 * SPDX-FileCopyrightText: 2021 권경환 Kyunghwan Kwon <k@libmcu.org>
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
