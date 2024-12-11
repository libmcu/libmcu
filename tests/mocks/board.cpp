#include "CppUTestExt/MockSupport.h"
#include "libmcu/board.h"

unsigned long board_get_time_since_boot_ms(void) {
	return mock().actualCall(__func__).returnUnsignedIntValue();
}

uint64_t board_get_time_since_boot_us(void) {
	return mock().actualCall(__func__).returnUnsignedIntValue();
}

void board_reboot(void) {
	mock().actualCall(__func__);
}

uint32_t board_random(void) {
	return mock().actualCall(__func__).returnUnsignedIntValue();
}

unsigned long board_get_current_stack_watermark(void) {
	return mock().actualCall(__func__).returnUnsignedIntValue();
}
