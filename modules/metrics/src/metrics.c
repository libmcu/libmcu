/*
 * SPDX-FileCopyrightText: 2021 권경환 Kyunghwan Kwon <k@libmcu.org>
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
	metric_value_t value;
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

static struct metrics *get_item_by_index(const metric_key_t index)
{
	return &metrics[index];
}

static metric_key_t get_index_by_key(const metric_key_t key)
{
	for (metric_key_t i = 0; i < METRICS_KEY_MAX; i++) {
		if (get_item_by_index(i)->key == key) {
			return i;
		}
	}

	assert(0);
	return 0;
}

static struct metrics *get_obj_from_key(const metric_key_t key)
{
	return get_item_by_index(get_index_by_key(key));
}

static bool is_metric_set(const struct metrics *p)
{
	return p->is_set;
}

static bool validate_metrics(void)
{
	struct metrics *p = get_item_by_index(METRICS_KEY_MAGIC);

	if (p->key != MAGIC_KEY || p->value != MAGIC_VALUE) {
		return false;
	}

	for (metric_key_t i = 0; i < METRICS_KEY_MAX; i++) {
		if (get_item_by_index(i)->key != i) {
			return false;
		}
	}

	return true;
}

static void set_metric_value(const metric_key_t key, const metric_value_t value)
{
	get_obj_from_key(key)->value = value;
	get_obj_from_key(key)->is_set = true;
}

static metric_value_t get_metric_value(const metric_key_t key)
{
	return get_obj_from_key(key)->value;
}

static void set_metric_value_if_min(const metric_key_t key,
		const metric_value_t value)
{
	const struct metrics *p = get_obj_from_key(key);

	if (is_metric_set(p)) {
		if (value < p->value) {
			set_metric_value(key, value);
		}
	} else {
		set_metric_value(key, value);
	}
}

static void set_metric_value_if_max(const metric_key_t key,
		const metric_value_t value)
{
	const struct metrics *p = get_obj_from_key(key);

	if (is_metric_set(p)) {
		if (value > p->value) {
			set_metric_value(key, value);
		}
	} else {
		set_metric_value(key, value);
	}
}

static void iterate_all(void (*callback_each)(const metric_key_t key,
				const metric_value_t value, void *ctx),
		void *ctx)
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

static size_t encode_all(uint8_t *buf, const size_t bufsize)
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

void metrics_set(const metric_key_t key, const metric_value_t val)
{
	metrics_lock();
	set_metric_value(key, val);
	metrics_unlock();
}

void metrics_set_if_min(const metric_key_t key, const metric_value_t val)
{
	metrics_lock();
	set_metric_value_if_min(key, val);
	metrics_unlock();
}

void metrics_set_if_max(const metric_key_t key, const metric_value_t val)
{
	metrics_lock();
	set_metric_value_if_max(key, val);
	metrics_unlock();
}

void metrics_set_max_min(const metric_key_t k_max, const metric_key_t k_min,
		const metric_value_t val)
{
	metrics_lock();
	set_metric_value_if_max(k_max, val);
	set_metric_value_if_min(k_min, val);
	metrics_unlock();
}

metric_value_t metrics_get(const metric_key_t key)
{
	metric_value_t value;

	metrics_lock();
	value = get_metric_value(key);
	metrics_unlock();

	return value;
}

void metrics_increase(const metric_key_t key)
{
	metrics_lock();
	set_metric_value(key, get_metric_value(key) + 1);
	metrics_unlock();
}

void metrics_increase_by(const metric_key_t key, const metric_value_t n)
{
	metrics_lock();
	set_metric_value(key, get_metric_value(key) + n);
	metrics_unlock();
}

bool metrics_is_set(const metric_key_t key)
{
	metrics_lock();
	const bool is_set = is_metric_set(get_obj_from_key(key));
	metrics_unlock();
	return is_set;
}

void metrics_reset(void)
{
	metrics_lock();
	reset_all();
	metrics_unlock();
}

void metrics_unset(const metric_key_t key)
{
	metrics_lock();
	struct metrics *p = get_obj_from_key(key);
	if (is_metric_set(p)) {
		p->value = 0;
		p->is_set = false;
	}
	metrics_unlock();
}

size_t metrics_collect(void *buf, const size_t bufsize)
{
	size_t written;

	metrics_lock();
	written = encode_all((uint8_t *)buf, bufsize);
	metrics_unlock();

	return written;
}

void metrics_iterate(void (*callback_each)(const metric_key_t key,
				const metric_value_t value, void *ctx),
		void *ctx)
{
	iterate_all(callback_each, ctx);
}

size_t metrics_count(void)
{
	return METRICS_KEY_MAX;
}

#if !defined(METRICS_NO_KEY_STRING)
const char *metrics_stringify_key(const metric_key_t key)
{
	return key_strings[key];
}
#endif

void metrics_init(const bool force)
{
	if (force || !validate_metrics()) {
		initialize_metrics();

		struct metrics *p = get_item_by_index(METRICS_KEY_MAGIC);
		p->key = MAGIC_KEY;
		p->value = MAGIC_VALUE;
	}
}
