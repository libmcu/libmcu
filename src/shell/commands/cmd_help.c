#include "commands.h"
#include <string.h>
#include "libmcu/shell.h"
#include "libmcu/compiler.h"

static shell_cmd_error_t cmd_help(int argc, const char *argv[], const void *env)
{
	unused(argc);
	unused(argv);
	unused(env);

	const shell_cmd_t **commands = shell_get_command_list();
	const shell_cmd_t *cmd;
	const shell_io_t *io = env;

	for (int i = 0; (cmd = commands[i]) != NULL; i++) {
		io->write(cmd->name, strlen(cmd->name));
		io->write("\t: ", 3);
		io->write(cmd->desc, strlen(cmd->desc));
		io->write("\r\n", 2);
	}

	return SHELL_CMD_SUCCESS;
}

const shell_cmd_t g_cmd_help = {
	.name = "help",
	.run = cmd_help,
	.desc = "Display available commands",
};
