#include "libmcu/timext.h"
#include <zephyr/kernel.h>

#define time_after(goal, chasing)	((int)(goal)    - (int)(chasing) < 0)

void timeout_set(unsigned int *goal, unsigned int msec)
{
	*goal = k_uptime_get_32() + msec;
}

bool timeout_is_expired(unsigned int goal)
{
	return time_after(goal, k_uptime_get_32());
}

void sleep_ms(unsigned int msec)
{
	k_sleep(K_MSEC(msec));
}
