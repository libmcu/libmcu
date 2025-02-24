/*
 * SPDX-FileCopyrightText: 2020 권경환 Kyunghwan Kwon <k@libmcu.org>
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
#include <stdint.h>
#include "cli_cmd.h"
#include "cli_overrides.h"

#if !defined(CLI_ASSERT)
#define CLI_ASSERT(exp)
#endif

struct cli_io {
	int (*read)(void *buf, size_t bufsize);
	int (*write)(void const *data, size_t datasize);
};

struct cli {
	struct cli_io const *io;
	struct cli_cmd const **cmdlist;

	uint16_t cursor_pos;
	uint16_t history_next;
	uint16_t history_active;
	char previous_input;

	char *buf;
	size_t bufsize;

	void *env;
};

/**
 * @brief Initialize the CLI module.
 *
 * This function initializes the CLI module with the provided I/O interface,
 * buffer, and environment.
 *
 * @param[in] cli Pointer to the CLI structure.
 * @param[in] io Pointer to the CLI I/O interface structure.
 * @param[in] buf Pointer to the buffer used for CLI input.
 * @param[in] bufsize Size of the buffer.
 * @param[in] env Pointer to the environment structure.
 */
void cli_init(struct cli *cli, struct cli_io const *io,
		void *buf, size_t bufsize, void *env);

/**
 * @brief Register a list of commands with the CLI.
 *
 * This function registers a list of commands that can be executed by the CLI.
 *
 * @param[in] cli Pointer to the CLI structure.
 * @param[in] cmdlist Pointer to the list of commands to register.
 */
void cli_register_cmdlist(struct cli *cli, const struct cli_cmd **cmdlist);

/**
 * @brief Run the CLI main loop.
 *
 * This function runs the main loop of the CLI, processing input and executing
 * commands. The provided sleep function is called to yield control between
 * iterations.
 *
 * @param[in] cli Pointer to the CLI structure.
 * @param[in] sleep Pointer to the sleep function.
 */
void cli_run(struct cli *cli, void (*sleep)(void));

/**
 * @brief Execute a single step of the CLI.
 *
 * This function executes a single step of the CLI, processing input and
 * executing commands as necessary.
 *
 * @param[in] cli Pointer to the CLI structure.
 *
 * @return cli_cmd_error_t Error code indicating the result of the command
 *         execution.
 */
cli_cmd_error_t cli_step(struct cli *cli);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_CLI_H */
