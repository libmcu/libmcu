#include "cli_commands.h"
#include <string.h>
#include "libmcu/cli.h"
#include "libmcu/system.h"

typedef enum {
	CMD_OPT_NONE			= 0x00,
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
	} else if (opt == NULL) {
		return CMD_OPT_NONE;
	} else if (strcmp(opt, "version") == 0) {
		return CMD_OPT_VERSION;
	} else if (strcmp(opt, "sn") == 0) {
		return CMD_OPT_SERIAL_NUMBER;
	} else if (strcmp(opt, "build") == 0) {
		return CMD_OPT_BUILD_DATE;
	}

	return CMD_OPT_ALL;
}

static void print_version(const cli_io_t *io)
{
	const char *ver = system_get_version_string();
	io->write(ver, strlen(ver));
	io->write("\r\n", 2);
}

static void print_sn(const cli_io_t *io)
{
	const char *sn = system_get_serial_number_string();
	io->write(sn, strlen(sn));
	io->write("\r\n", 2);
}

static void print_build_date(const cli_io_t *io)
{
	const char *build_date = system_get_build_date_string();
	io->write(build_date, strlen(build_date));
	io->write("\r\n", 2);
}

cli_cmd_error_t cli_cmd_info(int argc, const char *argv[], const void *env)
{
	if (argc > 2) {
		return CLI_CMD_INVALID_PARAM;
	}

	struct cli const *cli = (struct cli const *)env;

	cmd_opt_t options = get_command_option(argc, argv? argv[1] : NULL);

	if (options & CMD_OPT_VERSION) {
		print_version(cli->io);
	}
	if (options & CMD_OPT_SERIAL_NUMBER) {
		print_sn(cli->io);
	}
	if (options & CMD_OPT_BUILD_DATE) {
		print_build_date(cli->io);
	}

	return CLI_CMD_SUCCESS;
}
