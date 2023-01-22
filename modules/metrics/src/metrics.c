/*
 * SPDX-FileCopyrightText: 2021 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/metrics.h"
#include "libmcu/metrics_overrides.h"
#include "libmcu/compiler.h"
#include "libmcu/assert.h"

#define MAGIC_CODE			((uintptr_t)metrics)

enum {
#define METRICS_DEFINE(key)		METRICS_##key,
#include METRICS_USER_DEFINES
#undef METRICS_DEFINE
	METRICS_KEY_MAX,
};
LIBMCU_ASSERT(METRICS_KEY_MAX < (1U << sizeof(metric_key_t) * 8));

struct metrics {
	metric_key_t key;
	int32_t value;
} LIBMCU_PACKED;

LIBMCU_NOINIT static uintptr_t magic;
LIBMCU_NOINIT static struct metrics metrics[METRICS_KEY_MAX];

#if !defined(METRICS_NO_KEY_STRING)
static char const *key_strings[] = {
#define METRICS_DEFINE(keystr) #keystr,
#include METRICS_USER_DEFINES
#undef METRICS_DEFINE
};
#endif

static struct metrics *get_item_by_index(uint32_t index)
{
	return &metrics[index];
}

static uint32_t get_index_by_key(metric_key_t key)
{
	for (metric_key_t i = 0; i < METRICS_KEY_MAX; i++) {
		if (get_item_by_index(i)->key == key) {
			return i;
		}
	}

	assert(0);
	return 0;
}

static struct metrics *get_obj_from_key(metric_key_t key)
{
	return get_item_by_index(get_index_by_key(key));
}

static void iterate_all(void (*callback_each)(metric_key_t key, int32_t value,
					      void *ctx), void *ctx)
{
	for (metric_key_t i = 0; i < METRICS_KEY_MAX; i++) {
		struct metrics const *p = get_item_by_index(i);
		callback_each(p->key, p->value, ctx);
	}
}

static void reset_all(void)
{
	for (metric_key_t i = 0; i < METRICS_KEY_MAX; i++) {
		get_item_by_index(i)->key = i;
		get_item_by_index(i)->value = 0;
	}
}

static uint32_t count_metrics_with_nonzero_value(void)
{
	uint32_t nr_updated = 0;

	for (metric_key_t i = 0; i < METRICS_KEY_MAX; i++) {
		struct metrics const *p = get_item_by_index(i);
		if (p->value != 0) {
			nr_updated++;
		}
	}

	return nr_updated;
}

static size_t encode_all(uint8_t *buf, size_t bufsize)
{
	size_t written = metrics_encode_header(buf, bufsize,
			METRICS_KEY_MAX, count_metrics_with_nonzero_value());

	for (metric_key_t i = 0; i < METRICS_KEY_MAX; i++) {
		struct metrics const *p = get_item_by_index(i);
		written += metrics_encode_each(&buf[written], bufsize - written,
				p->key, p->value);
	}

	return written;
}

static void initialize_metrics(void)
{
	reset_all();
}

void metrics_set(metric_key_t key, int32_t val)
{
	metrics_lock();
	get_obj_from_key(key)->value = val;
	metrics_unlock();
}

int32_t metrics_get(metric_key_t key)
{
	int32_t value;

	metrics_lock();
	value = get_obj_from_key(key)->value;
	metrics_unlock();

	return value;
}

void metrics_increase(metric_key_t key)
{
	metrics_lock();
	get_obj_from_key(key)->value++;
	metrics_unlock();
}

void metrics_increase_by(metric_key_t key, int32_t n)
{
	metrics_lock();
	get_obj_from_key(key)->value += n;
	metrics_unlock();
}

void metrics_reset(void)
{
	metrics_lock();
	reset_all();
	metrics_unlock();
}

size_t metrics_collect(void *buf, size_t bufsize)
{
	size_t written;

	metrics_lock();
	written = encode_all((uint8_t *)buf, bufsize);
	metrics_unlock();

	return written;
}

void metrics_iterate(void (*callback_each)(metric_key_t key, int32_t value,
					   void *ctx), void *ctx)
{
	metrics_lock();
	iterate_all(callback_each, ctx);
	metrics_unlock();
}

size_t metrics_count(void)
{
	return METRICS_KEY_MAX;
}

#if !defined(METRICS_NO_KEY_STRING)
const char *metrics_stringify_key(metric_key_t key)
{
	return key_strings[key];
}
#endif

void metrics_init(bool force)
{
	if (force || magic != MAGIC_CODE) {
		initialize_metrics();
		magic = MAGIC_CODE;
	}
}
