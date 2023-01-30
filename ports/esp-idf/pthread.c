/*
 * SPDX-FileCopyrightText: 2021 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <pthread.h>
#include <errno.h>
#include <string.h>
#include "esp_pthread.h"

int pthread_attr_setschedparam(pthread_attr_t *attr,
		const struct sched_param *param)
{
	esp_pthread_cfg_t cfg = esp_pthread_get_default_config();

	cfg.prio = param->sched_priority;

	if (esp_pthread_set_cfg(&cfg) != ESP_OK) {
		return -ENOTSUP;
	}

	return 0;
}

int pthread_attr_getschedparam(const pthread_attr_t *attr,
		struct sched_param *param)
{
	memcpy(param, &attr->schedparam, sizeof(*param));

	esp_pthread_cfg_t cfg;
	if (esp_pthread_get_cfg(&cfg) != ESP_OK) {
		return -ENOTSUP;
	}
	param->sched_priority = cfg.prio;

	return 0;
}
