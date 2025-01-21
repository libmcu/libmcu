/*
 * SPDX-FileCopyrightText: 2023 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_RUNNER_OVERRIDES_H
#define LIBMCU_RUNNER_OVERRIDES_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief This function performs a step in the runner's execution and calculates
 *        the time until the next execution period. It should be called
 *        periodically in the main loop of the application.
 *
 * @param[out] until_next_period_ms A pointer to a uint32_t where the time until
 *             the next execution period will be stored.
 *
 * @return int Returns 0 if the step was successful, and a non-zero error code
 *             otherwise.
 */
int runner_step(uint32_t *until_next_period_ms);
void runner_run(void);
void runner_sleep(void);
bool runner_busy(void);
int runner_input(void *ctx);
int runner_output(void *ctx);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_RUNNER_OVERRIDES_H */
