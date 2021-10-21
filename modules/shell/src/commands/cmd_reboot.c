#include "commands.h"
#include "libmcu/compiler.h"
#include "libmcu/system.h"

shell_cmd_error_t shell_cmd_reboot(int argc, const char *argv[], const void *env)
{
	unused(argc);
	unused(argv);
	unused(env);

	system_reboot();

	return SHELL_CMD_SUCCESS;
}
