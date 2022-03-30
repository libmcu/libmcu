#include "libmcu/metrics.h"
#include <string.h>
#include "libmcu/compiler.h"
#include "libmcu/assert.h"

#define MAGIC_CODE			((uintptr_t)metrics)

enum {
#define METRICS_DEFINE(id, key)		METRICS_##key##id,
#include METRICS_USER_DEFINES
#undef METRICS_DEFINE
	METRICS_KEY_MAX,
};

struct metrics {
	metric_key_t key;
	int32_t value;
} LIBMCU_PACKED;

LIBMCU_NOINIT static uintptr_t magic;
LIBMCU_NOINIT static struct metrics metrics[METRICS_KEY_MAX];

#if defined(METRICS_KEY_STRING)
static char const *key_strings[] = {
#define METRICS_DEFINE(id, keystr) [id] = #keystr,
#include METRICS_USER_DEFINES
#undef METRICS_DEFINE
};
#endif

static struct metrics *get_obj_from_index(uint32_t index)
{
	return &metrics[index];
}

static uint32_t get_index_from_key(metric_key_t key)
{
	for (uint32_t i = 0; i < METRICS_KEY_MAX; i++) {
		if (get_obj_from_index(i)->key == key) {
			return i;
		}
	}

	assert(0);
	return 0;
}

static struct metrics *get_obj_from_key(metric_key_t key)
{
	return get_obj_from_index(get_index_from_key(key));
}

static void iterate_all(void (*callback_each)(metric_key_t key, int32_t value,
					      void *ctx), void *ctx)
{
	for (uint32_t i = 0; i < METRICS_KEY_MAX; i++) {
		struct metrics const *p = get_obj_from_index(i);
		callback_each(p->key, p->value, ctx);
	}
}

static void reset_all(void)
{
	for (uint32_t i = 0; i < METRICS_KEY_MAX; i++) {
		get_obj_from_index(i)->value = 0;
	}
}

static uint32_t count_metrics_with_nonzero_value(void)
{
	uint32_t nr_updated = 0;

	for (uint32_t i = 0; i < METRICS_KEY_MAX; i++) {
		struct metrics const *p = get_obj_from_index(i);
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

	for (uint32_t i = 0; i < METRICS_KEY_MAX; i++) {
		struct metrics const *p = get_obj_from_index(i);
		written += metrics_encode_each(&buf[written], bufsize - written,
				p->key, p->value);
	}

	return written;
}

static void initialize_metrics(void)
{
	struct metrics tmp[] = {
#define METRICS_DEFINE(id, keystr) (struct metrics){ .key = keystr, .value = 0 },
#include METRICS_USER_DEFINES
#undef METRICS_DEFINE
	};
	assert(sizeof(tmp) == sizeof(metrics));
	memcpy(metrics, tmp, sizeof(metrics));

	reset_all();
}

void metrics_set(metric_key_t key, int32_t val)
{
	metrics_lock();
	{
		get_obj_from_key(key)->value = val;
	}
	metrics_unlock();
}

int32_t metrics_get(metric_key_t key)
{
	int32_t value;

	metrics_lock();
	{
		value = get_obj_from_key(key)->value;
	}
	metrics_unlock();

	return value;
}

void metrics_increase(metric_key_t key)
{
	metrics_lock();
	{
		get_obj_from_key(key)->value++;
	}
	metrics_unlock();
}

void metrics_increase_by(metric_key_t key, int32_t n)
{
	metrics_lock();
	{
		get_obj_from_key(key)->value += n;
	}
	metrics_unlock();
}

void metrics_reset(void)
{
	metrics_lock();
	{
		reset_all();
	}
	metrics_unlock();
}

size_t metrics_collect(void *buf, size_t bufsize)
{
	size_t written;

	metrics_lock();
	{
		written = encode_all((uint8_t *)buf, bufsize);
	}
	metrics_unlock();

	return written;
}

void metrics_iterate(void (*callback_each)(metric_key_t key, int32_t value,
					   void *ctx), void *ctx)
{
	metrics_lock();
	{
		iterate_all(callback_each, ctx);
	}
	metrics_unlock();
}

size_t metrics_count(void)
{
	return METRICS_KEY_MAX;
}

#if defined(METRICS_KEY_STRING)
char const *metrics_stringify_key(metric_key_t key)
{
	return key_strings[key];
}
#endif

void metrics_init(bool force)
{
	metrics_lock_init();

	if (force || magic != MAGIC_CODE) {
		initialize_metrics();
		magic = MAGIC_CODE;
	}
}

LIBMCU_WEAK size_t metrics_encode_header(void *buf, size_t bufsize,
		uint32_t nr_total, uint32_t nr_updated)
{
	unused(buf);
	unused(bufsize);
	unused(nr_total);
	unused(nr_updated);
	return 0;
}

LIBMCU_WEAK size_t metrics_encode_each(void *buf, size_t bufsize,
		metric_key_t key, int32_t value)
{
	unused(key);

	if (bufsize < sizeof(value)) {
		return 0;
	}

	memcpy(buf, &value, sizeof(value));

	return sizeof(value);
}

LIBMCU_WEAK void metrics_lock_init(void)
{
}

LIBMCU_WEAK void metrics_lock(void)
{
}

LIBMCU_WEAK void metrics_unlock(void)
{
}
