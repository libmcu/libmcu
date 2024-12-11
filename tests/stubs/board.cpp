#include "CppUTestExt/MockSupport.h"
#include "libmcu/board.h"

unsigned long board_get_time_since_boot_ms(void) {
	return 0;
}

uint64_t board_get_time_since_boot_us(void) {
	return 0;
}

void board_reboot(void) {
	return;
}

uint32_t board_random(void) {
	return 0;
}

unsigned long board_get_current_stack_watermark(void) {
	return 0;
}
