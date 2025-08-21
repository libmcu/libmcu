/*
 * SPDX-FileCopyrightText: 2020 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/retry.h"
#include <stdlib.h>

#if !defined(RETRY_INFO)
#define RETRY_INFO(...)
#endif

static uint16_t get_jitter(const uint16_t max_jitter_ms,
		const uint16_t random_jitter)
{
	if (max_jitter_ms) {
		return random_jitter % max_jitter_ms;
	}
	return 0;
}

static void reset_backoff(struct retry *self)
{
	self->attempts = 0;
	self->previous_backoff_ms = 0;
}

static retry_error_t init(struct retry *self, const struct retry_param *param)
{
	if (!self || !param) {
		return RETRY_ERROR_INVALID_PARAM;
	}

	self->param = *param;

	if (self->param.min_backoff_ms > self->param.max_backoff_ms) {
		self->param.max_backoff_ms = self->param.min_backoff_ms;
	}

	reset_backoff(self);

	return RETRY_ERROR_NONE;
}

static bool is_exhausted(const struct retry *self)
{
	return self->param.max_attempts &&
		self->attempts >= self->param.max_attempts;
}

static uint32_t calc_backoff_time(const struct retry *self,
		const uint16_t random_jitter)
{
	const uint16_t jitter =
		get_jitter(self->param.max_jitter_ms, random_jitter);
	uint32_t backoff = self->previous_backoff_ms * 2 + jitter;

	if (self->previous_backoff_ms == 0) {
		backoff = (uint32_t)self->param.min_backoff_ms + jitter;
	}

	if (backoff > self->param.max_backoff_ms) {
		backoff = self->param.max_backoff_ms -
			self->param.max_jitter_ms + jitter;
	}

	return backoff;
}

static uint32_t update(struct retry *self, const uint16_t random_jitter)
{
	unsigned int backoff_time = calc_backoff_time(self, random_jitter);

	self->attempts++;
	self->previous_backoff_ms = backoff_time;

	return backoff_time;
}

retry_error_t retry_backoff(struct retry *self,
		uint32_t *next_backoff_ms, const uint16_t random_jitter)
{
	if (!self || !next_backoff_ms) {
		return RETRY_ERROR_INVALID_PARAM;
	}
	if (is_exhausted(self)) {
		return RETRY_ERROR_EXHAUSTED;
	}

	*next_backoff_ms = update(self, random_jitter);

	RETRY_INFO("Retry in %u ms, %u/%u", *next_backoff_ms,
			self->attempts, self->param.max_attempts);

	return RETRY_ERROR_NONE;
}

uint32_t retry_get_backoff(const struct retry *self)
{
	return self->previous_backoff_ms;
}

void retry_reset(struct retry *self)
{
	reset_backoff(self);
}

bool retry_exhausted(const struct retry *self)
{
	return is_exhausted(self);
}

bool retry_first(const struct retry *self)
{
	return self->attempts == 0;
}

retry_error_t retry_new_static(struct retry *self,
		const struct retry_param *param)
{
	return init(self, param);
}

struct retry *retry_new(const struct retry_param *param)
{
	struct retry *p = (struct retry *)malloc(sizeof(struct retry));
	if (!p) {
		return NULL;
	}

	if (init(p, param) != RETRY_ERROR_NONE) {
		free(p);
		return NULL;
	}

	return p;
}

void retry_delete(struct retry *self)
{
	free(self);
}
