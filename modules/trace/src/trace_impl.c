#include "libmcu/trace.h"
#include "libmcu/board.h"

LIBMCU_WEAK
LIBMCU_NO_INSTRUMENT
void trace_enter_hook(const struct trace *entry)
{
	unused(entry);
}

LIBMCU_WEAK
LIBMCU_NO_INSTRUMENT
void trace_leave_hook(const struct trace *entry)
{
	unused(entry);
}

LIBMCU_WEAK
LIBMCU_NO_INSTRUMENT
uint32_t trace_get_time(void)
{
	return (uint32_t)board_get_time_since_boot_ms();
}

LIBMCU_WEAK
LIBMCU_NO_INSTRUMENT
size_t trace_get_stack_watermark(void)
{
	return (size_t)board_get_current_stack_watermark();
}

LIBMCU_WEAK
LIBMCU_NO_INSTRUMENT
void *trace_get_current_thread(void)
{
	return board_get_current_thread();
}
