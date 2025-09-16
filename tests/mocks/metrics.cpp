#include "CppUTestExt/MockSupport.h"
#include "libmcu/metrics.h"

void metrics_increase(metric_key_t key) {
	mock().actualCall(__func__).withParameter("key", key);
}

void metrics_set(metric_key_t key, int32_t val) {
	mock().actualCall(__func__)
		.withParameter("key", key)
		.withParameter("val", val);
}

void metrics_unset(const metric_key_t key) {
	mock().actualCall(__func__).withParameter("key", key);
}
