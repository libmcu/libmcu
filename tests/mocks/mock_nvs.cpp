#include "CppUTestExt/MockSupport.h"

extern "C" {
#include "nvs.h"
#include "compiler.h"
}

extern "C" int nvs_set_blob(nvs_handle_t UNUSED c_handle,
		const char UNUSED *key, const void UNUSED *value,
		size_t UNUSED length)
{
	return mock().actualCall(__func__)
		.returnIntValueOrDefault(0);
}

extern "C" int nvs_commit(nvs_handle_t UNUSED c_handle)
{
	return mock().actualCall(__func__)
		.returnIntValueOrDefault(0);
}

extern "C" int nvs_get_blob(nvs_handle_t UNUSED c_handle,
		const char UNUSED *key, void UNUSED *out_value,
		size_t UNUSED *length)
{
	return mock().actualCall(__func__)
		.returnIntValueOrDefault(0);
}

extern "C" int nvs_open(const char UNUSED *name,
		nvs_open_mode_t UNUSED open_mode,
		nvs_handle_t UNUSED *out_handle)
{
	return mock().actualCall(__func__)
		.returnIntValueOrDefault(0);
}

extern "C" void nvs_close(nvs_handle_t UNUSED handle)
{
}

extern "C" int nvs_flash_init(void)
{
	return mock().actualCall(__func__)
		.returnIntValueOrDefault(0);
}
