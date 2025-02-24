/*
 * SPDX-FileCopyrightText: 2020 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_CLI_OVERRIDES_H
#define LIBMCU_CLI_OVERRIDES_H

#if defined(__cplusplus)
extern "C" {
#endif

struct cli;
struct cli_io;

const struct cli_io *cli_io_create(void);
void cli_start(struct cli *cli);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_CLI_OVERRIDES_H */
