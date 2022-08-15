#ifndef LIBMCU_CLI_CMDS_H
#define LIBMCU_CLI_CMDS_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "libmcu/cli_cmd.h"

cli_cmd_error_t cli_cmd_exit(int argc, const char *argv[], const void *env);
cli_cmd_error_t cli_cmd_help(int argc, const char *argv[], const void *env);
cli_cmd_error_t cli_cmd_info(int argc, const char *argv[], const void *env);
cli_cmd_error_t cli_cmd_reboot(int argc, const char *argv[], const void *env);
cli_cmd_error_t cli_cmd_memdump(int argc, const char *argv[], const void *env);

extern struct cli_cmd const cli_commands[];

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_CLI_CMDS_H */
