#include "CppUTestExt/MockSupport.h"
#include "libmcu/board.h"

uint32_t board_get_time_since_boot_ms(void) {
	return (uint32_t)mock().actualCall(__func__).returnUnsignedIntValue();
}

uint64_t board_get_time_since_boot_us(void) {
	return (uint64_t)mock().actualCall(__func__).returnUnsignedIntValue();
}

void board_reboot(void) {
	mock().actualCall(__func__);
}

uint32_t board_random(void) {
	return (uint32_t)mock().actualCall(__func__).returnUnsignedIntValue();
}

uint32_t board_get_current_stack_watermark(void) {
	return (uint32_t)mock().actualCall(__func__).returnUnsignedIntValue();
}

const char *board_get_serial_number_string(void) {
	return mock().actualCall(__func__).returnStringValue();
}
