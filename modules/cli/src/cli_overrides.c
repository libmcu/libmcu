/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/cli_overrides.h"
#include "libmcu/compiler.h"

LIBMCU_WEAK
const struct cli_io *cli_io_create(void)
{
	return NULL;
}
