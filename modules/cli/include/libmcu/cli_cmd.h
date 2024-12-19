/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_CLI_CMD_H
#define LIBMCU_CLI_CMD_H

#if defined(__cplusplus)
extern "C" {
#endif

#if !defined(CLI_CMD_MAXLEN)
#define CLI_CMD_MAXLEN				62
#endif
#if !defined(CLI_CMD_ARGS_MAXLEN)
#define CLI_CMD_ARGS_MAXLEN			8
#endif

typedef enum {
	CLI_CMD_SUCCESS				= 0,
	CLI_CMD_EXIT,
	CLI_CMD_NOT_FOUND,
	CLI_CMD_INVALID_PARAM,
	CLI_CMD_BLANK,
	CLI_CMD_ERROR,
} cli_cmd_error_t;

typedef cli_cmd_error_t (*cli_cmd_func_t)(int argc, const char *argv[],
		void *env);

struct cli_cmd {
	char const *name;
	cli_cmd_func_t func;
	char const *desc;
};

#include "cli_cmd_macro.h"

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_CLI_CMD_H */
