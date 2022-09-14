/*
 * SPDX-FileCopyrightText: 2021 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"

void *malloc(size_t size)
{
	if (size == 0) {
		return NULL;
	}

	return pvPortMalloc(size);
}

void free(void *ptr)
{
	if (ptr == NULL) {
		return;
	}

	vPortFree(ptr);
}
