/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_CLI_H
#define LIBMCU_CLI_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>
#include "cli_cmd.h"
#include "cli_overrides.h"

#if !defined(CLI_ASSERT)
#define CLI_ASSERT(exp)
#endif

struct cli_io {
	size_t (*read)(void *buf, size_t bufsize);
	size_t (*write)(void const *data, size_t datasize);
};

struct cli {
	struct cli_io const *io;
	struct cli_cmd const **cmdlist;
	char cmdbuf[CLI_CMD_MAXLEN + 1/*linefeed*/ + 1/*null*/];
	size_t cmdbuf_index;
};

void cli_init(struct cli *cli, struct cli_io const *io,
		const struct cli_cmd **cmdlist);
void cli_run(struct cli *cli);
void cli_step(struct cli *cli);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_CLI_H */
