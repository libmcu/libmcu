#include "CppUTest/TestHarness.h"
#include <string.h>
#include "libmcu/cli.h"

static cli_cmd_error_t cmd_exit(int argc, const char *argv[], const void *env)
{
	return CLI_CMD_EXIT;
}
static cli_cmd_error_t cmd_args(int argc, const char *argv[], const void *env)
{
	struct cli const *cli = (struct cli const *)env;
	char buf[1024];
	int len = 0;
	for (int i = 0; i < argc; i++) {
		int written = sprintf(buf+len, "%d: %s\n", i+1, argv[i]);
		len += written;
	}
	cli->io->write(buf, (size_t)len);
	return CLI_CMD_SUCCESS;
}
static cli_cmd_error_t cmd_error(int argc, const char *argv[], const void *env)
{
	return CLI_CMD_ERROR;
}
static cli_cmd_error_t cmd_invalid(int argc, const char *argv[], const void *env)
{
	return CLI_CMD_INVALID_PARAM;
}

static struct cli_cmd const commands[] = {
	{ "exit", cmd_exit, "Exit the CLI" },
	{ "args", cmd_args, "" },
	{ "error", cmd_error, NULL },
	{ "invalid", cmd_invalid, "desc" },
};

static char writebuf[256];
static char readbuf[256];
static size_t write_index;
static size_t read_index;

static size_t myread(void *buf, size_t bufsize) {
	memcpy(buf, writebuf + write_index, bufsize);
	write_index += bufsize;
	return bufsize;
}
static size_t mywrite(const void *data, size_t data_size) {
	memcpy(readbuf + read_index, data, data_size);
	read_index += data_size;
	readbuf[read_index] = '\0';
	return data_size;
}
static void set_write_data(const char *s) {
	strncpy(writebuf, s, sizeof(writebuf));
}

static cli_io_t io = {
	.read = myread,
	.write = mywrite,
};

TEST_GROUP(cli) {
	struct cli cli;

	void setup(void) {
		write_index = 0;
		read_index = 0;

		cli_init(&cli, &io, commands,
				sizeof(commands) / sizeof(commands[0]));
	}
	void teardown() {
	}
	void given(const char *s) {
		set_write_data(s);
	}
	void then(const char *s) {
		STRNCMP_EQUAL(s, &readbuf[51], strlen(s));
	}
};

TEST(cli, cli_ShouldReturnUnknownCommand_WhenUnknownCommandGiven) {
	given("Hello, World\nexit\n");
	cli_run(&cli);
	then("$ Hello, World\ncommand not found\n");
}

TEST(cli, cli_ShouldReturnUnknownCommand_WhenUnknownCommandGivenWithCR) {
	given("Hello, World\rexit\r");
	cli_run(&cli);
	then("$ Hello, World\ncommand not found\n");
}

TEST(cli, cli_ShouldReturnUnknownCommand_WhenUnknownCommandGivenWithCRLF) {
	given("Hello, World\nexit\n");
	cli_run(&cli);
	then("$ Hello, World\ncommand not found\n");
}

TEST(cli, cli_ShouldReturnExit_WhenExitCommandGiven) {
	given("exit\n");
	cli_run(&cli);
	then("$ exit\nEXIT\n");
}

TEST(cli, cli_ShouldReturnBLANK_WhenNoCommandGiven) {
	given("\nexit\n");
	cli_run(&cli);
	then("$ \n$ exit\nEXIT\n");
}

TEST(cli, cli_ShouldDeletePreviousCharacter_WhenBackspaceGiven) {
	given("help\bq\nexit\n");
	cli_run(&cli);
	then("$ help\b \bq\ncommand not found");
}

TEST(cli, cli_ShouldIgnoreTab) {
	given("hel\tp\nexit\n");
	cli_run(&cli);
	then("$ help\n");
}

TEST(cli, cli_ShouldParseArgs_WhenMultipleArgsGiven) {
	given("args 1 2 3\nexit\n");
	cli_run(&cli);
	then("$ args 1 2 3\n1: args\n2: 1\n3: 2\n4: 3\n");
}

TEST(cli, cli_ShouldIgnoreArgs_WhenMoreThanMaxArgsGiven) {
	given("args 1 2 3 4\nexit\n");
	cli_run(&cli);
	then("$ args 1 2 3 4\n1: args\n2: 1\n3: 2\n4: 3\n");
}

TEST(cli, cli_ShouldReturnError_WhenErrorGiven) {
	given("error\nexit\n");
	cli_run(&cli);
	then("$ error\nERROR\n");
}

TEST(cli, cli_ShouldReturnDesc_WhenCommandUsageInvalid) {
	given("invalid\nexit\n");
	cli_run(&cli);
	then("$ invalid\ndesc\n");
}

TEST(cli, cli_ShouldIgnoreInput_WhenDefaultMaxLen62Reached) {
	given("1234567890123456789012345678901234567890123456789012345678901234567890\nexit\n");
	cli_run(&cli);
	then("$ 12345678901234567890123456789012345678901234567890123456789012\n");
}
