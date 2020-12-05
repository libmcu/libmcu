#include "commands.h"
#include "libmcu/compiler.h"

static shell_cmd_error_t cmd_exit(int argc, const char *argv[], const void *env)
{
	unused(argc);
	unused(argv);
	unused(env);

	return SHELL_CMD_EXIT;
}

shell_cmd_t g_cmd_exit = {
	.name = "exit",
	.run = cmd_exit,
	.desc = "Exit the shell",
};
