/*
 * SPDX-FileCopyrightText: 2023 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

/* NOTE: All the functions in this module are not thread-safe. */

#ifndef LIBMCU_RUNNER_H
#define LIBMCU_RUNNER_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <errno.h>

#include "libmcu/runner_overrides.h"

typedef uint8_t runner_t;
typedef bool (*runner_change_cb_t)(const runner_t type, void *ctx);

struct runner_api {
	int (*prepare)(void);
	void (*terminate)(void);
	int (*step)(uint32_t *until_next_period_ms);
	void (*run)(void);
	void (*sleep)(void);
	bool (*busy)(void);
	int (*input)(void *input_ctx);
	int (*output)(void *output_ctx);
};

struct runner {
	const struct runner_api *api;
	const runner_t type;
};

/**
 * @brief This function initializes the runner module with an array of runners.
 *
 * @param runners[in] A pointer to the first element of an array of `struct
 *                    runner` that represents the runners to be managed by the
 *                    module.
 * @param nr_runners[in] The number of runners in the array.
 *
 * @return int Returns 0 if the initialization was successful, and a non-zero
 *             error code otherwise.
 */
int runner_init(const struct runner *runners, size_t nr_runners);

/**
 * @breif This function registers callback functions that are called before and
 *        after the runner changes its state.
 *
 * @param pre_cb[in] A pointer to the function to be called before the runner
 *                   changes its state.
 * @param pre_ctx[in] A void pointer that will be passed as the argument to the
 *                    pre_cb function.
 * @param post_cb[in] A pointer to the function to be called after the runner
 *                    changes its state.
 * @param post_ctx[in] A void pointer that will be passed as the argument to the
 *                     post_cb function.
 *
 * @return int Returns 0 if the callbacks were successfully registered, and a
 *             non-zero error code otherwise.
 */
int runner_register_change_cb(runner_change_cb_t pre_cb, void *pre_ctx,
		runner_change_cb_t post_cb, void *post_ctx);
void runner_start(const runner_t runner_type);
int runner_change(const runner_t new_runner_type);
const struct runner *runner_current(void);
runner_t runner_type(const struct runner *runner);
runner_t runner_current_type(void);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_RUNNER_H */
