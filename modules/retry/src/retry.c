/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/retry.h"
#include "libmcu/system.h"

#if !defined(RETRY_DEFAULT_MAX_BACKOFF_MS)
#define RETRY_DEFAULT_MAX_BACKOFF_MS		300000U // 5-min
#endif
#if !defined(RETRY_DEFAULT_MIN_BACKOFF_MS)
#define RETRY_DEFAULT_MIN_BACKOFF_MS		5000U // 5-sec
#endif
#if !defined(RETRY_DEFAULT_MAX_JITTER_MS)
#define RETRY_DEFAULT_MAX_JITTER_MS		RETRY_DEFAULT_MIN_BACKOFF_MS
#endif

static uint16_t get_jitter(uint16_t max_jitter_ms)
{
	return (uint16_t)system_random() % max_jitter_ms;
}

static void reset_backoff(struct retry_params *param)
{
	param->attempts = 0;
	param->previous_backoff_ms = 0;
}

static unsigned int get_backoff_time(const struct retry_params *param)
{
	uint16_t jitter = get_jitter(param->max_jitter_ms);

	unsigned int backoff;
	if (param->previous_backoff_ms == 0) {
		backoff = (unsigned int)param->min_backoff_ms + jitter;
	} else {
		backoff = param->previous_backoff_ms * 2 + jitter;
	}

	if (backoff > param->max_backoff_ms) {
		backoff = param->max_backoff_ms - param->max_jitter_ms + jitter;
	}

	return backoff;
}

static void adjust_params(struct retry_params *param)
{
	if (param->max_backoff_ms == 0) {
		param->max_backoff_ms = RETRY_DEFAULT_MAX_BACKOFF_MS;
	}
	if (param->max_jitter_ms == 0) {
		param->max_jitter_ms = param->min_backoff_ms > 0?
			param->min_backoff_ms : RETRY_DEFAULT_MAX_JITTER_MS;
	}
}

retry_error_t retry_backoff(struct retry_params *param)
{
	adjust_params(param);

	if (param->attempts >= param->max_attempts) {
		reset_backoff(param);
		return RETRY_EXHAUSTED;
	}

	unsigned int backoff_time = get_backoff_time(param);
	RETRY_DEBUG("Retry in %u ms, %u/%u", backoff_time,
			param->attempts+1, param->max_attempts);
	param->sleep(backoff_time);
	param->attempts++;
	param->previous_backoff_ms = backoff_time;

	return RETRY_SUCCESS;
}

void retry_reset(struct retry_params *param)
{
	reset_backoff(param);
}
