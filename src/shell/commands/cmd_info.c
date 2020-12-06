#include "commands.h"
#include <string.h>
#include "libmcu/shell.h"
#include "libmcu/system.h"

typedef enum {
	CMD_OPT_VERSION			= 0x01,
	CMD_OPT_BUILD_DATE		= 0x02,
	CMD_OPT_SERIAL_NUMBER		= 0x04,
	CMD_OPT_ALL			= (
			CMD_OPT_VERSION |
			CMD_OPT_BUILD_DATE |
			CMD_OPT_SERIAL_NUMBER),
} cmd_opt_t;

static cmd_opt_t get_command_option(int argc, const char *opt)
{
	if (argc == 1) {
		return CMD_OPT_ALL;
	} else if (strcmp(opt, "version") == 0) {
		return CMD_OPT_VERSION;
	} else if (strcmp(opt, "sn") == 0) {
		return CMD_OPT_SERIAL_NUMBER;
	} else if (strcmp(opt, "build") == 0) {
		return CMD_OPT_BUILD_DATE;
	}

	return CMD_OPT_ALL;
}

static void print_version(const shell_io_t *io)
{
	const char *ver = system_get_version_string();
	io->write(ver, strlen(ver));
}

static void print_sn(const shell_io_t *io)
{
	const char *sn = system_get_serial_number_string();
	io->write(sn, strlen(sn));
}

static void print_build_date(const shell_io_t *io)
{
	const char *build_date = system_get_build_date_string();
	io->write(build_date, strlen(build_date));
}

shell_cmd_error_t shell_cmd_info(int argc, const char *argv[], const void *env)
{
	if (argc > 2) {
		return SHELL_CMD_INVALID_PARAM;
	}

	const shell_io_t *io = env;

	cmd_opt_t options = get_command_option(argc, argv? argv[1] : NULL);

	if (options & CMD_OPT_VERSION) {
		print_version(io);
	}
	if (options & CMD_OPT_SERIAL_NUMBER) {
		print_sn(io);
	}
	if (options & CMD_OPT_BUILD_DATE) {
		print_build_date(io);
	}

	return SHELL_CMD_SUCCESS;
}
