#ifndef LIBMCU_SYSTEM_H
#define LIBMCU_SYSTEM_H 1

#if defined(__cplusplus)
extern "C" {
#endif

void system_reboot(void);
void system_reset_factory(void);
const char *system_get_serial_number_string(void);
const char *system_get_version_string(void);
const char *system_get_build_date_string(void);
const char *system_get_reboot_reason_string(void);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_SYSTEM_H */
