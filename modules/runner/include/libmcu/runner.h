/*
 * SPDX-FileCopyrightText: 2023 권경환 Kyunghwan Kwon <k@libmcu.org>
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
	int (*prepare)(void *ctx);
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
 * @param[in] runners A pointer to the first element of an array of `struct
 *                    runner` that represents the runners to be managed by the
 *                    module.
 * @param[in] nr_runners The number of runners in the array.
 * @param[in] ctx The context of runner
 *
 * @return int Returns 0 if the initialization was successful, and a non-zero
 *             error code otherwise.
 */
int runner_init(const struct runner *runners, size_t nr_runners, void *ctx);

/**
 * @breif This function registers callback functions that are called before and
 *        after the runner changes its state.
 *
 * @param[in] pre_cb A pointer to the function to be called before the runner
 *                   changes its state.
 * @param[in] pre_ctx A void pointer that will be passed as the argument to the
 *                    pre_cb function.
 * @param[in] post_cb A pointer to the function to be called after the runner
 *                    changes its state.
 * @param[in] post_ctx A void pointer that will be passed as the argument to the
 *                     post_cb function.
 *
 * @return int Returns 0 if the callbacks were successfully registered, and a
 *             non-zero error code otherwise.
 */
int runner_register_change_cb(runner_change_cb_t pre_cb, void *pre_ctx,
		runner_change_cb_t post_cb, void *post_ctx);

/**
 * @brief Start the specified runner.
 *
 * This function starts the runner of the specified type.
 *
 * @param[in] runner_type The type of the runner to start.
 */
void runner_start(const runner_t runner_type);

/**
 * @brief Change the runner type with pre/post callbacks and prepare/terminate.
 *
 * This function changes the current runner type to the specified new runner
 * type. It will call the pre-change and post-change callbacks if they are
 * defined, and it will also call the runner's prepare and terminate functions.
 *
 * @param[in] new_runner_type The new runner type to switch to.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int runner_change(const runner_t new_runner_type);

/**
 * @brief Change the runner type without pre/post callbacks and without
 * prepare/terminate.
 *
 * This function changes the current runner type to the specified new runner
 * type without calling any pre-change or post-change callbacks, and without
 * calling the runner's prepare or terminate functions.
 *
 * @param[in] new_runner_type The new runner type to switch to.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int runner_change_bypass(const runner_t new_runner_type);

/**
 * @brief Get the current runner.
 *
 * This function returns a pointer to the current runner.
 *
 * @return A pointer to the current runner.
 */
const struct runner *runner_current(void);

/**
 * @brief Get the type of the specified runner.
 *
 * This function returns the type of the specified runner.
 *
 * @param[in] runner A pointer to the runner.
 *
 * @return The type of the specified runner.
 */
runner_t runner_type(const struct runner *runner);

/**
 * @brief Get the type of the current runner.
 *
 * This function returns the type of the current runner.
 *
 * @return The type of the current runner.
 */
runner_t runner_current_type(void);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_RUNNER_H */
