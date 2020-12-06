#include "commands.h"
#include <string.h>
#include "libmcu/compiler.h"
#include "libmcu/shell.h"

static shell_cmd_error_t cmd_version(int argc, const char *argv[], const void *env)
{
	unused(argc);
	unused(argv);

	const shell_io_t *io = env;

	io->write(def2str(VERSION), strlen(def2str(VERSION)));

	return SHELL_CMD_SUCCESS;
}

const shell_cmd_t g_cmd_version = {
	.name = "version",
	.run = cmd_version,
	.desc = "Display the version",
};
