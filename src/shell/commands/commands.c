#include "commands.h"
#include <stddef.h>

static const shell_cmd_t commands[] = {
	{"exit", shell_cmd_exit, "Exit the shell" },
	{"help", shell_cmd_help, "List available commands" },
	{"info", shell_cmd_info, "Get device info" },
	{"reboot", shell_cmd_reboot, "Reboot the device" },
	{ NULL, NULL, NULL },
};

const shell_cmd_t *shell_get_command_list(void)
{
	return commands;
}
