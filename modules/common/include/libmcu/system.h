#ifndef LIBMCU_SYSTEM_H
#define LIBMCU_SYSTEM_H

#if defined(__cplusplus)
extern "C" {
#endif

void system_init(void);
void system_reboot(void);
int system_reset_factory(void);

const char *system_get_version_string(void);
const char *system_get_build_date_string(void);

const char *system_get_serial_number_string(void);
const char *system_get_reboot_reason_string(void);

unsigned int system_get_free_heap_bytes(void);
unsigned int system_get_total_heap_bytes(void);
unsigned int system_get_heap_watermark(void);
unsigned int system_get_current_stack_watermark(void);

int system_random(void);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_SYSTEM_H */
