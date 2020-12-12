#include "libmcu/system.h"
#include <assert.h>
#include "libmcu/compiler.h"

const char *system_get_version_string(void)
{
	return def2str(VERSION);
}

const char *system_get_build_date_string(void)
{
	return BUILD_DATE;
}

const char * LIBMCU_WEAK system_get_serial_number_string(void)
{
	return "S/N";
}

const char * LIBMCU_WEAK system_get_reboot_reason_string(void)
{
	return "N/A";
}

void LIBMCU_WEAK system_reset_factory(void)
{
}

void LIBMCU_WEAK LIBMCU_NORETURN system_reboot(void)
{
	assert(0);
}
