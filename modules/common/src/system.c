/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/system.h"
#include "libmcu/compiler.h"
#include "libmcu/assert.h"

const char * LIBMCU_WEAK system_get_version_string(void)
{
	return def2str(VERSION);
}

const char * LIBMCU_WEAK system_get_build_date_string(void)
{
#if defined(BUILD_DATE)
	return BUILD_DATE;
#else
	return __DATE__;
#endif
}

const char * LIBMCU_WEAK system_get_serial_number_string(void)
{
	return "S/N";
}

const char * LIBMCU_WEAK system_get_reboot_reason_string(void)
{
	return "N/A";
}

int LIBMCU_WEAK system_reset_factory(void)
{
	return 0;
}

void LIBMCU_WEAK system_reboot(void)
{
	assert(0);
}

void LIBMCU_WEAK system_init(void)
{
}

unsigned int LIBMCU_WEAK system_get_free_heap_bytes(void)
{
	return 0;
}

unsigned int LIBMCU_WEAK system_get_heap_watermark(void)
{
	return 0;
}

unsigned int LIBMCU_WEAK system_get_current_stack_watermark(void)
{
	return 0;
}

int LIBMCU_WEAK system_random(void)
{
	return 0;
}
