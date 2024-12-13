/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/ratelim.h"

static void update_bucket(struct ratelim *bucket)
{
	time_t now = time(NULL);
	uint32_t elapsed = (uint32_t)(now - bucket->last_update);
	uint32_t leaked = elapsed * bucket->leak_rate;

	if (leaked >= bucket->current_load) {
		bucket->current_load = 0;
	} else {
		bucket->current_load -= leaked;
	}

	bucket->last_update = now;
}

static bool is_bucket_full(const struct ratelim *bucket)
{
	return bucket->current_load >= bucket->capacity;
}

static bool request_token(struct ratelim *bucket)
{
	update_bucket(bucket);

	if (is_bucket_full(bucket)) {
		return false;
	}

	bucket->current_load++;

	return true;
}

bool ratelim_full(struct ratelim *bucket)
{
	update_bucket(bucket);
	return is_bucket_full(bucket);
}

bool ratelim_request(struct ratelim *bucket)
{
	return request_token(bucket);
}

bool ratelim_request_format(struct ratelim *bucket,
		ratelim_format_func_t func, const char *format, ...)
{
	if (!request_token(bucket)) {
		return false;
	}

	if (func) {
		va_list args;
		va_start(args, format);
		(*func)(format, args);
		va_end(args);
	}

	return true;
}

void ratelim_init(struct ratelim *bucket,
		const uint32_t cap, const uint32_t leak_rate)
{
	*bucket = (struct ratelim) {
		.capacity = cap,
		.current_load = 0,
		.leak_rate = leak_rate,
		.last_update = time(NULL),
	};
}
