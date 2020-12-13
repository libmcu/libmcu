#include "CppUTestExt/MockSupport.h"
#include "nvs.h"
#include "libmcu/compiler.h"

extern "C" int nvs_set_blob(nvs_handle_t LIBMCU_UNUSED c_handle,
		const char LIBMCU_UNUSED *key, const void LIBMCU_UNUSED *value,
		size_t LIBMCU_UNUSED length)
{
	return mock().actualCall(__func__)
		.returnIntValueOrDefault(0);
}

extern "C" int nvs_commit(nvs_handle_t LIBMCU_UNUSED c_handle)
{
	return mock().actualCall(__func__)
		.returnIntValueOrDefault(0);
}

extern "C" int nvs_get_blob(nvs_handle_t LIBMCU_UNUSED c_handle,
		const char LIBMCU_UNUSED *key, void LIBMCU_UNUSED *out_value,
		size_t LIBMCU_UNUSED *length)
{
	return mock().actualCall(__func__)
		.returnIntValueOrDefault(0);
}

extern "C" int nvs_open(const char LIBMCU_UNUSED *name,
		nvs_open_mode_t LIBMCU_UNUSED open_mode,
		nvs_handle_t LIBMCU_UNUSED *out_handle)
{
	return mock().actualCall(__func__)
		.returnIntValueOrDefault(0);
}

extern "C" void nvs_close(nvs_handle_t LIBMCU_UNUSED handle)
{
}

extern "C" int nvs_flash_init(void)
{
	return mock().actualCall(__func__)
		.returnIntValueOrDefault(0);
}
