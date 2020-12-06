#include "CppUTest/TestHarness.h"
#include <string.h>
#include "libmcu/shell.h"
#include "../src/shell/shell_command.h"

static shell_cmd_error_t cmd_exit(int argc, const char *argv[], const void *env)
{
	return SHELL_CMD_EXIT;
}
const shell_cmd_t g_cmd_exit = {
	.name = "exit",
	.run = cmd_exit,
	.desc = "Exit the shell",
};
static shell_cmd_error_t cmd_args(int argc, const char *argv[], const void *env)
{
	const shell_io_t *io = (const shell_io_t *)env;
	char buf[1024];
	int len = 0;
	for (int i = 0; i < argc; i++) {
		int written = sprintf(buf+len, "%d: %s\n", i+1, argv[i]);
		len += written;
	}
	io->write(buf, (size_t)len);
	return SHELL_CMD_SUCCESS;
}
const shell_cmd_t g_cmd_args = {
	.name = "args",
	.run = cmd_args,
	.desc = "",
};
static shell_cmd_error_t cmd_error(int argc, const char *argv[], const void *env)
{
	return SHELL_CMD_ERROR;
}
const shell_cmd_t g_cmd_error = {
	.name = "error",
	.run = cmd_error,
	.desc = NULL,
};
static shell_cmd_error_t cmd_invalid(int argc, const char *argv[], const void *env)
{
	return SHELL_CMD_INVALID_PARAM;
}
const shell_cmd_t g_cmd_invalid = {
	.name = "invalid",
	.run = cmd_invalid,
	.desc = "desc",
};

static const shell_cmd_t *commands[] = {
	&g_cmd_exit,
	&g_cmd_args,
	&g_cmd_error,
	&g_cmd_invalid,
	NULL,
};
const shell_cmd_t **shell_get_command_list(void)
{
	return (const shell_cmd_t **)commands;
}

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

static shell_io_t io = {
	.read = myread,
	.write = mywrite,
};

TEST_GROUP(shell) {
	void setup(void) {
		write_index = 0;
		read_index = 0;
	}
	void teardown() {
	}
	void given(const char *s) {
		set_write_data(s);
	}
	void then(const char *s) {
		STRNCMP_EQUAL(s, &readbuf[54], strlen(s));
	}
};

TEST(shell, shell_ShouldReturnUnknownCommand_WhenUnknownCommandGiven) {
	given("Hello, World\nexit\n");
	shell_run(&io);
	then("$ Hello, World\r\ncommand not found\r\n");
}

TEST(shell, shell_ShouldReturnUnknownCommand_WhenUnknownCommandGivenWithCR) {
	given("Hello, World\rexit\r");
	shell_run(&io);
	then("$ Hello, World\r\ncommand not found\r\n");
}

TEST(shell, shell_ShouldReturnUnknownCommand_WhenUnknownCommandGivenWithCRLF) {
	given("Hello, World\r\nexit\r\n");
	shell_run(&io);
	then("$ Hello, World\r\ncommand not found\r\n");
}

TEST(shell, shell_ShouldReturnExit_WhenExitCommandGiven) {
	given("exit\r\n");
	shell_run(&io);
	then("$ exit\r\nEXIT\r\n");
}

TEST(shell, shell_ShouldReturnBLANK_WhenNoCommandGiven) {
	given("\nexit\n");
	shell_run(&io);
	then("$ \r\n$ exit\r\nEXIT\r\n");
}

TEST(shell, shell_ShouldDeletePreviousCharacter_WhenBackspaceGiven) {
	given("help\bq\nexit\n");
	shell_run(&io);
	then("$ help\bq\r\ncommand not found");
}

TEST(shell, shell_ShouldIgnoreTab) {
	given("hel\tp\nexit\n");
	shell_run(&io);
	then("$ help\r\n");
}

TEST(shell, shell_ShouldParseArgs_WhenMultipleArgsGiven) {
	given("args 1 2 3 4\nexit\n");
	shell_run(&io);
	then("$ args 1 2 3 4\r\n1: args\n2: 1\n3: 2\n4: 3\n5: 4\n");
}

TEST(shell, shell_ShouldReturnError_WhenErrorGiven) {
	given("error\nexit\n");
	shell_run(&io);
	then("$ error\r\nERROR\r\n");
}

TEST(shell, shell_ShouldReturnDesc_WhenCommandUsageInvalid) {
	given("invalid\nexit\n");
	shell_run(&io);
	then("$ invalid\r\ndesc\r\n");
}
