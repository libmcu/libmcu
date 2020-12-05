#include "commands.h"
#include <stddef.h>

static const shell_cmd_t *commands[] = {
	&g_cmd_exit,
	&g_cmd_help,
	NULL,
};

const shell_cmd_t **shell_get_command_list(void)
{
	return (const shell_cmd_t **)commands;
}
