#include "CppUTestExt/MockSupport.h"
#include "libmcu/board.h"

unsigned long board_get_time_since_boot_ms(void) {
	return mock().actualCall(__func__).returnUnsignedIntValue();
}

void board_reboot(void) {
	mock().actualCall(__func__);
}
