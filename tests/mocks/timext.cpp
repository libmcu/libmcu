#include "CppUTestExt/MockSupport.h"
#include "libmcu/timext.h"

bool timeout_is_expired(unsigned long goal) {
	return (bool)mock().actualCall(__func__).returnIntValue();
}

void timeout_set(unsigned long *goal, unsigned long msec) {
	mock().actualCall(__func__)
		.withParameter("msec", msec);
}
