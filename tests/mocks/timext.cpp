#include "CppUTestExt/MockSupport.h"
#include "libmcu/timext.h"

bool timeout_is_expired(uint32_t goal) {
	return (bool)mock().actualCall(__func__).returnIntValue();
}

void timeout_set(uint32_t *goal, uint32_t msec) {
	mock().actualCall(__func__).withParameter("msec", msec);
}

void sleep_ms(uint32_t msec) {
	mock().actualCall(__func__).withParameter("msec", msec);
}
