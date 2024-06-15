/*
 * SPDX-FileCopyrightText: 2021 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

void *malloc(size_t size)
{
	if (size == 0) {
		return NULL;
	}

	return pvPortMalloc(size);
}

void *calloc(size_t num, sizeo_t size)
{
	if (num == 0 || size == 0) {
		return NULL;
	}

	void *p = pvPortMalloc(num *size);

	if (p) {
		memset(p, 0, num * size);
	}

	return p;
}

void free(void *ptr)
{
	if (ptr == NULL) {
		return;
	}

	vPortFree(ptr);
}
