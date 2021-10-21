#include "commands.h"
#include <string.h>
#include "libmcu/cli.h"
#include "libmcu/compiler.h"

cli_cmd_error_t cli_cmd_help(int argc, const char *argv[], const void *env)
{
	unused(argc);
	unused(argv);
	unused(env);

	const cli_cmd_t *commands = cli_get_command_list();
	const cli_cmd_t *cmd;
	const cli_io_t *io = (const cli_io_t *)env;

	for (int i = 0; (cmd = &commands[i]) && cmd->name; i++) {
		io->write(cmd->name, strlen(cmd->name));
		if (cmd->desc) {
			io->write("\t: ", 3);
			io->write(cmd->desc, strlen(cmd->desc));
		}
		io->write("\r\n", 2);
	}

	return CLI_CMD_SUCCESS;
}
