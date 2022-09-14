/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "CppUTestExt/MockSupport.h"
#include <pthread.h>

#if 0
void pthread_exit(void *value_ptr)
{
	mock().actualCall(__func__);
}
#endif

int pthread_attr_init(pthread_attr_t *attr)
{
	return mock().actualCall(__func__)
		.returnIntValueOrDefault(0);
}

int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize)
{
	return mock().actualCall(__func__)
		.returnIntValueOrDefault(0);
}

int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate)
{
	return mock().actualCall(__func__)
		.returnIntValueOrDefault(0);
}

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
		void *(*start_routine)(void *), void *arg)
{
	mock().actualCall(__func__);

	if (!mock().hasReturnValue()) {
		start_routine(arg);
	}

	return mock().intReturnValue();
}
