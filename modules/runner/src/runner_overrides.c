/*
 * SPDX-FileCopyrightText: 2023 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/runner_overrides.h"
#include "libmcu/runner.h"
#include "libmcu/compiler.h"

LIBMCU_WEAK
int runner_step(uint32_t *until_next_period_ms)
{
	if (runner_current()->api->step) {
		return runner_current()->api->step(until_next_period_ms);
	}
	return -ENOENT;
}

LIBMCU_WEAK
void runner_run(void)
{
	if (runner_current()->api->run) {
		runner_current()->api->run();
	}
}

LIBMCU_WEAK
void runner_sleep(void)
{
	if (runner_current()->api->sleep) {
		runner_current()->api->sleep();
	}
}

LIBMCU_WEAK
bool runner_busy(void)
{
	if (runner_current()->api->busy) {
		return runner_current()->api->busy();
	}
	return false;
}

LIBMCU_WEAK
int runner_input(void *ctx)
{
	if (runner_current()->api->input) {
		return runner_current()->api->input(ctx);
	}
	return -ENOENT;
}

LIBMCU_WEAK
int runner_output(void *ctx)
{
	if (runner_current()->api->output) {
		return runner_current()->api->output(ctx);
	}
	return -ENOENT;
}
