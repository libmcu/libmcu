#include "commands.h"
#include <string.h>
#include "libmcu/compiler.h"
#include "libmcu/shell.h"

shell_cmd_error_t shell_cmd_info(int argc, const char *argv[], const void *env)
{
	unused(argc);
	unused(argv);

	const shell_io_t *io = env;

	io->write(def2str(VERSION), strlen(def2str(VERSION)));
	// "$ info sn" S/N
	// "$ info version" device version(soft, hard, app, boot)
	// "$ info build" build date
	// "$ info" all info. including reboot reason

	return SHELL_CMD_SUCCESS;
}
