/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "CppUTest/TestHarness.h"
#include <string.h>
#include "libmcu/cli.h"

#define CTRL_C 0x03
#define CTRL_P 0x10

DEFINE_CLI_CMD(exit, "Exit the CLI") {
	return CLI_CMD_EXIT;
}
DEFINE_CLI_CMD(args, "") {
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
DEFINE_CLI_CMD(error, NULL) {
	return CLI_CMD_ERROR;
}
DEFINE_CLI_CMD(invalid, "desc") {
	return CLI_CMD_INVALID_PARAM;
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

static struct cli_io io = {
	.read = myread,
	.write = mywrite,
};

TEST_GROUP(cli) {
	struct cli cli;
	char cli_buffer[CLI_CMD_MAXLEN * 4];

	void setup(void) {
		write_index = 0;
		read_index = 0;

		DEFINE_CLI_CMD_LIST(cmd_list, exit, args, error, invalid);
		cli_init(&cli, &io, cli_buffer, sizeof(cli_buffer));
		cli_register_cmdlist(&cli, cmd_list);
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

TEST(cli, ShouldNotSplitArguments_WhenQuotedWordGiven) {
	given("args first \"2 second\"\nexit\n");
	cli_run(&cli);
	then("$ args first \"2 second\"\n1: args\n2: first\n3: 2 second\n");
}

TEST(cli, ShouldTrimTrailingSpaces) {
	given("args first   \nexit\n");
	cli_run(&cli);
	then("$ args first   \n1: args\n2: first\n");
}

TEST(cli, ShouldTrimParameterLeadingSpaces) {
	given("args     first\nexit\n");
	cli_run(&cli);
	then("$ args     first\n1: args\n2: first\n");
}

TEST(cli, ShouldTrimLeadingSpaces) {
	given("  args     first\nexit\n");
	cli_run(&cli);
	then("$   args     first\n1: args\n2: first\n");
}

TEST(cli, ShouldPrintPreviousHistory_WhenCtrlPGiven) {
	char input[] = { 'a','r','g','s','\n', CTRL_P,'\n', 'e','x','i','t','\n' };
	given(input);
	cli_run(&cli);
	then("$ args\n1: args\n$ args\n1: args\n");
}
TEST(cli, ShouldPrintPreviousHistory_WhenCtrlPAfterTypingGiven) {
	char input[] = { 'a','r','g','s','\n', 't',CTRL_P,'\n', 'e','x','i','t','\n' };
	given(input);
	cli_run(&cli);
	then("$ args\n1: args\n$ t\bargs\n1: args\n");
}
TEST(cli, ShouldPrintPreviousHistory_WhenCtrlPAfterLongerTypingGiven) {
	char input[] = { 'a','r','g','s','\n', '1','2','3','4','5','6',CTRL_P,'\n', 'e','x','i','t','\n' };
	given(input);
	cli_run(&cli);
	then("$ args\n1: args\n$ 123456\b\b\b\b\b\bargs  \b\b\n1: args\n");
}
TEST(cli, ShouldPrintPreviousHistory_WhenOnlyOneHistoryGiven) {
	char input[] = { 'a','r','g','s','\n', CTRL_P, CTRL_P,'\n', 'e','x','i','t','\n' };
	given(input);
	cli_run(&cli);
	then("$ args\n1: args\n$ args\n1: args\n");
}

TEST(cli, ShouldPrintNewLine_WhenCtrlCGiven) {
	char input[] = { 'a','r','g', CTRL_C, 'e','x','i','t','\n' };
	given(input);
	cli_run(&cli);
	then("$ arg\n");
}
