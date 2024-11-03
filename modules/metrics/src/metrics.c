/*
 * SPDX-FileCopyrightText: 2021 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/metrics.h"
#include "libmcu/metrics_overrides.h"
#include "libmcu/compiler.h"
#include "libmcu/assert.h"

enum {
#define METRICS_DEFINE(key)		METRICS_##key,
#include METRICS_USER_DEFINES
#undef METRICS_DEFINE
	METRICS_KEY_MAX,
};
static_assert(METRICS_KEY_MAX < (1U << sizeof(metric_key_t) * 8),
	"METRICS_KEY_MAX must be less than the maximum value of metric_key_t");

#define METRICS_KEY_MAGIC		METRICS_KEY_MAX
#define MAGIC_KEY			0xffU
#define MAGIC_VALUE			((int32_t)(intptr_t)metrics)

struct metrics {
	metric_key_t key;
	int32_t value;
	bool is_set;
} LIBMCU_PACKED;

LIBMCU_NOINIT static struct metrics metrics[METRICS_KEY_MAX+1/*magic*/];

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

static bool is_metric_set(const struct metrics *p)
{
	return p->is_set;
}

static void set_metric_value(metric_key_t key, int32_t value)
{
	get_obj_from_key(key)->value = value;
	get_obj_from_key(key)->is_set = true;
}

static int32_t get_metric_value(metric_key_t key)
{
	return get_obj_from_key(key)->value;
}

static void iterate_all(void (*callback_each)(metric_key_t key, int32_t value,
					      void *ctx), void *ctx)
{
	for (metric_key_t i = 0; i < METRICS_KEY_MAX; i++) {
		struct metrics const *p = get_item_by_index(i);
		if (is_metric_set(p)) {
			callback_each(p->key, p->value, ctx);
		}
	}
}

static void reset_all(void)
{
	for (metric_key_t i = 0; i < METRICS_KEY_MAX; i++) {
		get_item_by_index(i)->key = i;
		get_item_by_index(i)->value = 0;
		get_item_by_index(i)->is_set = false;
	}
}

static uint32_t count_metrics_updated(void)
{
	uint32_t nr_updated = 0;

	for (metric_key_t i = 0; i < METRICS_KEY_MAX; i++) {
		struct metrics const *p = get_item_by_index(i);
		if (is_metric_set(p)) {
			nr_updated++;
		}
	}

	return nr_updated;
}

static size_t encode_all(uint8_t *buf, size_t bufsize)
{
	size_t written = metrics_encode_header(buf, bufsize,
			METRICS_KEY_MAX, count_metrics_updated());

	for (metric_key_t i = 0; i < METRICS_KEY_MAX; i++) {
		struct metrics const *p = get_item_by_index(i);
		if (is_metric_set(p)) {
			written += metrics_encode_each(&buf[written],
					bufsize - written, p->key, p->value);
		}
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
	set_metric_value(key, val);
	metrics_unlock();
}

int32_t metrics_get(metric_key_t key)
{
	int32_t value;

	metrics_lock();
	value = get_metric_value(key);
	metrics_unlock();

	return value;
}

void metrics_increase(metric_key_t key)
{
	metrics_lock();
	set_metric_value(key, get_metric_value(key) + 1);
	metrics_unlock();
}

void metrics_increase_by(metric_key_t key, int32_t n)
{
	metrics_lock();
	set_metric_value(key, get_metric_value(key) + n);
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
	iterate_all(callback_each, ctx);
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
	struct metrics *p = get_item_by_index(METRICS_KEY_MAGIC);

	if (force || p->key != MAGIC_KEY || p->value != MAGIC_VALUE) {
		initialize_metrics();
		p->key = MAGIC_KEY;
		p->value = MAGIC_VALUE;
	}
}
