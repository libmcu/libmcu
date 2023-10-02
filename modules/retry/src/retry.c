/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/retry.h"
#include "libmcu/retry_overrides.h"

static uint16_t get_jitter(uint16_t max_jitter_ms)
{
	if (max_jitter_ms) {
		return (uint16_t)retry_generate_random() % max_jitter_ms;
	}
	return 0;
}

static void reset_backoff(struct retry *self)
{
	self->attempts = 0;
	self->previous_backoff_ms = 0;
}

static bool is_exhausted(const struct retry *self)
{
	return self->attempts >= self->max_attempts;
}

static uint32_t calc_backoff_time(const struct retry *self)
{
	uint16_t jitter = get_jitter(self->max_jitter_ms);
	uint32_t backoff = self->previous_backoff_ms * 2 + jitter;

	if (self->previous_backoff_ms == 0) {
		backoff = (uint32_t)self->min_backoff_ms + jitter;
	}

	if (backoff > self->max_backoff_ms) {
		backoff = self->max_backoff_ms - self->max_jitter_ms + jitter;
	}

	return backoff;
}

static uint32_t next_backoff(struct retry *self)
{
	unsigned int backoff_time = calc_backoff_time(self);

	self->attempts++;
	self->previous_backoff_ms = backoff_time;

	return backoff_time;
}

retry_error_t retry_backoff(struct retry *self)
{
	if (is_exhausted(self)) {
		return RETRY_EXHAUSTED;
	}

	unsigned int backoff_time = next_backoff(self);
	RETRY_DEBUG("Retry in %u ms, %u/%u", backoff_time,
			self->attempts, self->max_attempts);

	if (backoff_time) {
		retry_sleep_ms(backoff_time);
	}

	return RETRY_RUNNING;
}

void retry_reset(struct retry *self)
{
	reset_backoff(self);
}

bool retry_exhausted(const struct retry *self)
{
	return is_exhausted(self);
}

uint32_t retry_backoff_next(struct retry *self)
{
	return next_backoff(self);
}

void retry_init(struct retry *self, uint16_t max_attempts,
		uint32_t max_backoff_ms, uint16_t min_backoff_ms,
		uint16_t max_jitter_ms)
{
	if (min_backoff_ms > max_backoff_ms) {
		max_backoff_ms = min_backoff_ms;
	}

	*self = (struct retry) {
		.max_backoff_ms = max_backoff_ms,
		.min_backoff_ms = min_backoff_ms,
		.max_jitter_ms = max_jitter_ms,
		.max_attempts = max_attempts,
	};

	reset_backoff(self);
}
