#include "libmcu/metrics.h"
#include <assert.h>
#include <pthread.h>

#define ARRAY_LEN(arr)		(sizeof(arr) / sizeof(arr[0]))
#define METRICS_LEN		ARRAY_LEN(metrics)

struct metrics_s {
	uint8_t id;
	int32_t value;
};

static struct metrics_s metrics[] = {
#define METRICS_DEFINE(_id, _key) (struct metrics_s){ .id = _id, .value = 0, },
#include "libmcu/metrics_defaults.def"
#undef METRICS_DEFINE
};

static pthread_mutex_t metrics_lock;

static struct metrics_s *get_obj_from_index(uint32_t index)
{
	return &metrics[index];
}

static uint32_t get_index_from_key(metric_key_t keyid)
{
	for (uint32_t i = 0; i < METRICS_LEN; i++) {
		if (get_obj_from_index(i)->id == keyid) {
			return i;
		}
	}

	assert(0);
}

static struct metrics_s *get_obj_from_key(metric_key_t keyid)
{
	return get_obj_from_index(get_index_from_key(keyid));
}

static void iterate_all(void (*callback_each)(metric_key_t keyid, int32_t value))
{
	for (uint32_t i = 0; i < METRICS_LEN; i++) {
		struct metrics_s *p = get_obj_from_index(i);
		callback_each(p->id, p->value);
	}
}

static void reset_all(void)
{
	for (uint32_t i = 0; i < METRICS_LEN; i++) {
		get_obj_from_index(i)->value = 0;
	}
}

void metrics_set(metric_key_t keyid, int32_t val)
{
	pthread_mutex_lock(&metrics_lock);
	{
		get_obj_from_key(keyid)->value = val;
	}
	pthread_mutex_unlock(&metrics_lock);
}

void metrics_increase(metric_key_t keyid)
{
	pthread_mutex_lock(&metrics_lock);
	{
		get_obj_from_key(keyid)->value++;
	}
	pthread_mutex_unlock(&metrics_lock);
}

void metrics_increase_by(metric_key_t keyid, int32_t n)
{
	pthread_mutex_lock(&metrics_lock);
	{
		get_obj_from_key(keyid)->value += n;
	}
	pthread_mutex_unlock(&metrics_lock);
}

void metrics_reset(void)
{
	pthread_mutex_lock(&metrics_lock);
	{
		reset_all();
	}
	pthread_mutex_unlock(&metrics_lock);
}

void metrics_iterate(void (*callback_each)(metric_key_t keyid, int32_t value))
{
	iterate_all(callback_each);
}

void metrics_init(void)
{
	reset_all();
	int rc = pthread_mutex_init(&metrics_lock, NULL);
	assert(rc == 0);
}
