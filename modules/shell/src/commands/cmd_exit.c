#include "commands.h"
#include "libmcu/compiler.h"

shell_cmd_error_t shell_cmd_exit(int argc, const char *argv[], const void *env)
{
	unused(argc);
	unused(argv);
	unused(env);

	return SHELL_CMD_EXIT;
}
