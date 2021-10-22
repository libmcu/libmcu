#include "cli_commands.h"
#include "libmcu/compiler.h"

cli_cmd_error_t cli_cmd_exit(int argc, const char *argv[], const void *env)
{
	unused(argc);
	unused(argv);
	unused(env);

	return CLI_CMD_EXIT;
}
