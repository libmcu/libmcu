/*
 * SPDX-FileCopyrightText: 2024 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/ratelim.h"

#define SECONDS_PER_MINUTE	60
#define SECONDS_PER_HOUR	(60 * SECONDS_PER_MINUTE)
#define SCALE_TO(x)		((x) * SECONDS_PER_HOUR)
#define SCALE_FROM(x)		((x) / SECONDS_PER_HOUR)

static void update_bucket(struct ratelim *bucket)
{
	const time_t now = time(NULL);
	const uint32_t elapsed = (uint32_t)(now - bucket->last_update);

	if (now == (time_t)-1 || elapsed == 0) {
		return;
	}

	bucket->leak_time_buffer += elapsed;
	const uint32_t leaked =
		SCALE_FROM(bucket->leak_time_buffer * bucket->leak_rate);

	if (leaked) {
		if (leaked >= bucket->current_load) {
			bucket->current_load = 0;
		} else {
			bucket->current_load -= leaked;
		}
		bucket->leak_time_buffer -=
			SCALE_TO(leaked) / bucket->leak_rate;
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

bool ratelim_request_ext(struct ratelim *bucket, const uint32_t n)
{
	if (n == 0) {
		return true;
	}

	if (n > bucket->capacity) {
		return false;
	}

	update_bucket(bucket);

	if (bucket->current_load + n > bucket->capacity) {
		return false;
	}

	bucket->current_load += n;

	return true;
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

void ratelim_init(struct ratelim *bucket, const ratelim_unit_t unit,
		const uint32_t cap, const uint32_t leak_rate)
{
	uint32_t leak_rate_per_unit = leak_rate;

	switch (unit) {
	case RATELIM_UNIT_SECOND:
		leak_rate_per_unit *= SECONDS_PER_HOUR;
		break;
	case RATELIM_UNIT_MINUTE:
		leak_rate_per_unit *= SECONDS_PER_MINUTE;
		break;
	default:
		break;
	}

	*bucket = (struct ratelim) {
		.capacity = cap,
		.current_load = 0,
		.leak_rate = leak_rate_per_unit,
		.leak_time_buffer = 0,
		.last_update = time(NULL),
	};
}
