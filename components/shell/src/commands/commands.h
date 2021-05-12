#ifndef LIBMCU_SHELL_CMDS_H
#define LIBMCU_SHELL_CMDS_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "libmcu/shell_command.h"

shell_cmd_error_t shell_cmd_exit(int argc, const char *argv[], const void *env);
shell_cmd_error_t shell_cmd_help(int argc, const char *argv[], const void *env);
shell_cmd_error_t shell_cmd_info(int argc, const char *argv[], const void *env);
shell_cmd_error_t shell_cmd_reboot(int argc, const char *argv[], const void *env);
shell_cmd_error_t shell_cmd_memdump(int argc, const char *argv[], const void *env);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_SHELL_CMDS_H */
