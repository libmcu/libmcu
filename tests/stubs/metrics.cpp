#include "CppUTestExt/MockSupport.h"
#include "libmcu/metrics.h"

void metrics_set(const metric_key_t key, const metric_value_t val) {
	(void)key;
	(void)val;
}

void metrics_set_if_min(const metric_key_t key, const metric_value_t val) {
	(void)key;
	(void)val;
}

void metrics_set_if_max(const metric_key_t key, const metric_value_t val) {
	(void)key;
	(void)val;
}

void metrics_set_max_min(const metric_key_t k_max, const metric_key_t k_min,
		const metric_value_t val) {
	(void)k_max;
	(void)k_min;
	(void)val;
}

void metrics_unset(const metric_key_t key) {
	(void)key;
}

metric_value_t metrics_get(const metric_key_t key) {
	(void)key;
	return 0;
}

void metrics_increase(const metric_key_t key) {
	(void)key;
}

void metrics_increase_by(const metric_key_t key, const metric_value_t n) {
	(void)key;
	(void)n;
}

void metrics_reset(void) {
}

bool metrics_is_set(const metric_key_t key) {
	(void)key;
	return false;
}

void metrics_iterate(void (*callback_each)(const metric_key_t key,
				const metric_value_t value, void *ctx),
		void *ctx) {
	(void)callback_each;
	(void)ctx;
}

size_t metrics_collect(void *buf, const size_t bufsize) {
	(void)buf;
	(void)bufsize;
	return 0;
}

size_t metrics_count(void) {
	return 0;
}

void metrics_init(const bool force) {
	(void)force;
}

const char *metrics_stringify_key(const metric_key_t key) {
	(void)key;
	return NULL;
}
