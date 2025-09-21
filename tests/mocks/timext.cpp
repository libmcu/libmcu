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

int iso8601_convert_to_string(time_t t, char *buf, size_t bufsize) {
	return mock().actualCall(__func__)
		.withParameter("t", t)
		.withParameter("buf", buf)
		.withParameter("bufsize", bufsize)
		.returnIntValueOrDefault(0); // Success
}

#if defined(_GNU_SOURCE)
time_t iso8601_convert_to_time(const char *tstr) {
	return (time_t)mock().actualCall(__func__)
		.withParameter("tstr", tstr)
		.returnIntValueOrDefault(1640995200); // Default: 2022-01-01T00:00:00Z
}
#endif
