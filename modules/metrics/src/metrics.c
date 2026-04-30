/*
 * SPDX-FileCopyrightText: 2021 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/metrics.h"
#include "libmcu/metrics_overrides.h"
#include "libmcu/compiler.h"
#include "libmcu/assert.h"

#define METRICS_FIRST_ARG(first, ...)		first
#define METRICS_ENUM_KEY_(key)			METRICS_##key
#define METRICS_ENUM_KEY(key)			METRICS_ENUM_KEY_(key)

enum {
#define METRICS_DEFINE(key)			METRICS_ENUM_KEY(key),
#define METRICS_DEFINE_COUNTER(key)		METRICS_DEFINE(key)
#define METRICS_DEFINE_GAUGE(key, mn, mx)	METRICS_DEFINE(key)
#define METRICS_DEFINE_PERCENTAGE(key)		METRICS_DEFINE(key)
#define METRICS_DEFINE_TIMER(key, u)		METRICS_DEFINE(key)
#define METRICS_DEFINE_BYTES(key)		METRICS_DEFINE(key)
#define METRICS_DEFINE_BINARY(key)		METRICS_DEFINE(key)
#define METRICS_DEFINE_STATE(...)		\
	METRICS_DEFINE(METRICS_FIRST_ARG(__VA_ARGS__, keep_at_least_one_arg))
#include METRICS_USER_DEFINES
#undef METRICS_DEFINE
#undef METRICS_DEFINE_COUNTER
#undef METRICS_DEFINE_GAUGE
#undef METRICS_DEFINE_PERCENTAGE
#undef METRICS_DEFINE_TIMER
#undef METRICS_DEFINE_BYTES
#undef METRICS_DEFINE_BINARY
#undef METRICS_DEFINE_STATE
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

#if defined(METRICS_SCHEMA_IBS)
static const struct metric_schema schema_table[] = {
#define METRICS_DEFINE(key) \
	{ METRIC_CLASS_UNTYPED, METRIC_UNIT_NONE, INT32_MIN, INT32_MAX },
#define METRICS_DEFINE_COUNTER(key) \
	{ METRIC_CLASS_COUNTER, METRIC_UNIT_NONE, 0, INT32_MAX },
#define METRICS_DEFINE_GAUGE(key, mn, mx) \
	{ METRIC_CLASS_GAUGE, METRIC_UNIT_NONE, (mn), (mx) },
#define METRICS_DEFINE_PERCENTAGE(key) \
	{ METRIC_CLASS_PERCENTAGE, METRIC_UNIT_NONE, 0, 100 },
#define METRICS_DEFINE_TIMER(key, u) \
	{ METRIC_CLASS_TIMER, METRIC_UNIT_##u, 0, INT32_MAX },
#define METRICS_DEFINE_BYTES(key) \
	{ METRIC_CLASS_BYTES, METRIC_UNIT_NONE, 0, INT32_MAX },
#define METRICS_DEFINE_BINARY(key) \
	{ METRIC_CLASS_BINARY, METRIC_UNIT_NONE, 0, 1 },
#define METRICS_DEFINE_STATE(...) \
	{ METRIC_CLASS_STATE, METRIC_UNIT_NONE, INT32_MIN, INT32_MAX },
#include METRICS_USER_DEFINES
#undef METRICS_DEFINE
#undef METRICS_DEFINE_COUNTER
#undef METRICS_DEFINE_GAUGE
#undef METRICS_DEFINE_PERCENTAGE
#undef METRICS_DEFINE_TIMER
#undef METRICS_DEFINE_BYTES
#undef METRICS_DEFINE_BINARY
#undef METRICS_DEFINE_STATE
};

static void assert_value_in_schema_range(const metric_key_t key,
		const metric_value_t value)
{
	const struct metric_schema *s = &schema_table[key];
	assert(value >= s->range_min && value <= s->range_max);
}
#else
#define assert_value_in_schema_range(key, value)  ((void)0)
#endif /* METRICS_SCHEMA_IBS */

#if !defined(METRICS_NO_KEY_STRING)
#define METRICS_STRING_KEY_(key)		#key
#define METRICS_STRING_KEY(key)			METRICS_STRING_KEY_(key)
static char const *key_strings[] = {
#define METRICS_DEFINE(key)			METRICS_STRING_KEY(key),
#define METRICS_DEFINE_COUNTER(key)		METRICS_DEFINE(key)
#define METRICS_DEFINE_GAUGE(key, mn, mx)	METRICS_DEFINE(key)
#define METRICS_DEFINE_PERCENTAGE(key)		METRICS_DEFINE(key)
#define METRICS_DEFINE_TIMER(key, u)		METRICS_DEFINE(key)
#define METRICS_DEFINE_BYTES(key)		METRICS_DEFINE(key)
#define METRICS_DEFINE_BINARY(key)		METRICS_DEFINE(key)
#define METRICS_DEFINE_STATE(...)		\
	METRICS_DEFINE(METRICS_FIRST_ARG(__VA_ARGS__, keep_at_least_one_arg))
#include METRICS_USER_DEFINES
#undef METRICS_DEFINE
#undef METRICS_DEFINE_COUNTER
#undef METRICS_DEFINE_GAUGE
#undef METRICS_DEFINE_PERCENTAGE
#undef METRICS_DEFINE_TIMER
#undef METRICS_DEFINE_BYTES
#undef METRICS_DEFINE_BINARY
#undef METRICS_DEFINE_STATE
};
#undef METRICS_STRING_KEY
#undef METRICS_STRING_KEY_
#endif
#undef METRICS_ENUM_KEY
#undef METRICS_ENUM_KEY_
#undef METRICS_FIRST_ARG

static bool is_valid_key(const metric_key_t key)
{
	return key < METRICS_KEY_MAX;
}

static struct metrics *get_item_by_index(const metric_key_t index)
{
	return &metrics[index];
}

static struct metrics *get_obj_from_key(const metric_key_t key)
{
	assert(is_valid_key(key));
	return get_item_by_index(key);
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
	assert_value_in_schema_range(key, value);
	struct metrics *p = get_obj_from_key(key);
	p->value  = value;
	p->is_set = true;
}

static metric_value_t get_metric_value(const metric_key_t key)
{
	return get_obj_from_key(key)->value;
}

static void set_metric_value_if_min(const metric_key_t key,
		const metric_value_t value)
{
	const struct metrics *p = get_obj_from_key(key);

	if (!is_metric_set(p) || value < p->value) {
		set_metric_value(key, value);
	}
}

static void set_metric_value_if_max(const metric_key_t key,
		const metric_value_t value)
{
	const struct metrics *p = get_obj_from_key(key);

	if (!is_metric_set(p) || value > p->value) {
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

static size_t encode_all(uint8_t *buf, const size_t bufsize, void *ctx)
{
	size_t written = metrics_encode_header(buf, bufsize,
			METRICS_KEY_MAX, count_metrics_updated(), ctx);

	for (metric_key_t i = 0; i < METRICS_KEY_MAX; i++) {
		struct metrics const *p = get_item_by_index(i);
		if (is_metric_set(p)) {
			uint8_t *dst = buf ? &buf[written] : NULL;
			size_t remaining = (buf && bufsize > written)
					? bufsize - written : 0;
#if defined(METRICS_SCHEMA_IBS)
			written += metrics_encode_each(dst, remaining, p->key,
					p->value, &schema_table[i], ctx);
#else
			written += metrics_encode_each(dst, remaining,
					p->key, p->value, ctx);
#endif
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
	if (!is_valid_key(key)) {
		return;
	}
	metrics_lock();
	set_metric_value(key, val);
	metrics_unlock();
}

void metrics_set_if_min(const metric_key_t key, const metric_value_t val)
{
	if (!is_valid_key(key)) {
		return;
	}
	metrics_lock();
	set_metric_value_if_min(key, val);
	metrics_unlock();
}

void metrics_set_if_max(const metric_key_t key, const metric_value_t val)
{
	if (!is_valid_key(key)) {
		return;
	}
	metrics_lock();
	set_metric_value_if_max(key, val);
	metrics_unlock();
}

void metrics_set_max_min(const metric_key_t k_max, const metric_key_t k_min,
		const metric_value_t val)
{
	if (!is_valid_key(k_max) || !is_valid_key(k_min)) {
		return;
	}
	metrics_lock();
	set_metric_value_if_max(k_max, val);
	set_metric_value_if_min(k_min, val);
	metrics_unlock();
}

metric_value_t metrics_get(const metric_key_t key)
{
	metric_value_t value;

	if (!is_valid_key(key)) {
		return 0;
	}
	metrics_lock();
	value = get_metric_value(key);
	metrics_unlock();

	return value;
}

void metrics_increase(const metric_key_t key)
{
	if (!is_valid_key(key)) {
		return;
	}
	metrics_lock();
	set_metric_value(key, get_metric_value(key) + 1);
	metrics_unlock();
}

void metrics_increase_by(const metric_key_t key, const metric_value_t n)
{
	if (!is_valid_key(key)) {
		return;
	}
	metrics_lock();
	set_metric_value(key, get_metric_value(key) + n);
	metrics_unlock();
}

void metrics_set_pct(const metric_key_t key, const metric_value_t num,
		const metric_value_t denom)
{
	if (!is_valid_key(key) || denom == 0) {
		return;
	}
	metrics_lock();
	set_metric_value(key, (metric_value_t)((int64_t)num * 100 / denom));
	metrics_unlock();
}

bool metrics_is_set(const metric_key_t key)
{
	if (!is_valid_key(key)) {
		return false;
	}
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
	if (!is_valid_key(key)) {
		return;
	}
	metrics_lock();
	struct metrics *p = get_obj_from_key(key);
	if (is_metric_set(p)) {
		p->value = 0;
		p->is_set = false;
	}
	metrics_unlock();
}

size_t metrics_collect(void *buf, const size_t bufsize, void *ctx)
{
	size_t written;

	metrics_lock();
	written = encode_all((uint8_t *)buf, bufsize, ctx);
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

size_t metrics_count_set(void)
{
	metrics_lock();
	const uint32_t n = count_metrics_updated();
	metrics_unlock();
	return n;
}

#if !defined(METRICS_NO_KEY_STRING)
const char *metrics_stringify_key(const metric_key_t key)
{
	if (!is_valid_key(key)) {
		return "";
	}
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
