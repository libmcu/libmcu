/*
 * SPDX-FileCopyrightText: 2020 권경환 Kyunghwan Kwon <k@libmcu.org>
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
