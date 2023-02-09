/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "CppUTest/TestHarness.h"
#include <string.h>
#include <stdlib.h>
#include "libmcu/board.h"
#include "libmcu/cli.h"

const char *board_get_version_string(void) {
	return "version";
}
const char *board_get_build_date_string(void) {
	return "build date";
}
const char *board_get_serial_number_string(void) {
	return "serial number";
}

static char write_spy_buffer[1024];
static size_t write_spy_buffer_index;
static int write_spy(const void *data, size_t data_size) {
	memcpy(write_spy_buffer + write_spy_buffer_index, data, data_size);
	write_spy_buffer_index += data_size;
	write_spy_buffer[write_spy_buffer_index] = '\0';
	return (int)data_size;
}
static struct cli_io io = {
	.read = NULL,
	.write = write_spy,
};

DEFINE_CLI_CMD_LIST(cli_commands, exit, info, md);

TEST_GROUP(cli_commands) {
	struct cli cli;

	void setup(void) {
		write_spy_buffer_index = 0;
		(void)cli_commands;

		cli.io = &io;
	}
	void teardown() {
	}
};

TEST(cli_commands, exit_ShouldReturnShellCmdExit) {
	LONGS_EQUAL(CLI_CMD_EXIT, cli_cmd_exit.func(1, NULL, NULL));
}

TEST(cli_commands, info_ShouldReturnAllInfo_WhenNoArgsGiven) {
	LONGS_EQUAL(CLI_CMD_SUCCESS, cli_cmd_info.func(1, NULL, &cli));
	STRCMP_EQUAL("version\nserial number\nbuild date\n", write_spy_buffer);
}

TEST(cli_commands, info_ShouldReturnInvalidParam_WhenMoreThan2ArgsGiven) {
	LONGS_EQUAL(CLI_CMD_INVALID_PARAM, cli_cmd_info.func(3, NULL, &cli));
}

TEST(cli_commands, info_ShouldReturnVersion_WhenSecondArgumentGivenAsVersion) {
	const char *argv[] = { "info", "version", };
	LONGS_EQUAL(CLI_CMD_SUCCESS, cli_cmd_info.func(2, argv, &cli));
	STRCMP_EQUAL("version\n", write_spy_buffer);
}

TEST(cli_commands, info_ShouldReturnSerialNumber_WhenSecondArgumentGivenAsSn) {
	const char *argv[] = { "info", "sn", };
	LONGS_EQUAL(CLI_CMD_SUCCESS, cli_cmd_info.func(2, argv, &cli));
	STRCMP_EQUAL("serial number\n", write_spy_buffer);
}

TEST(cli_commands, info_ShouldReturnBuildDate_WhenSecondArgumentGiven) {
	const char *argv[] = { "info", "build", };
	LONGS_EQUAL(CLI_CMD_SUCCESS, cli_cmd_info.func(2, argv, &cli));
	STRCMP_EQUAL("build date\n", write_spy_buffer);
}

TEST_GROUP(memdump) {
	uint8_t memsrc[1024];
	char addr[32];
	struct cli cli;

	void setup(void) {
		cli.io = &io;

		write_spy_buffer_index = 0;
		for (int i = 0; i < (int)sizeof(memsrc); i++) {
			memsrc[i] = (uint8_t)(i + 20);
		}
		sprintf(addr, "%p", memsrc);
	}
	void teardown() {
	}

	void clear_spy_buffer(void) {
		write_spy_buffer_index = 0;
	}
};

TEST(memdump, md_ShouldReturnOneByte_WhenLengthOneGiven) {
	char fixed_mem[128];
	sprintf(fixed_mem, "%16p:  14                    ", (uintptr_t *)memsrc);
	const char *argv[] = { "md", addr, "1", };
	LONGS_EQUAL(CLI_CMD_SUCCESS, cli_cmd_md.func(3, argv, &cli));
	STRNCMP_EQUAL(fixed_mem, write_spy_buffer, strlen(fixed_mem));
}

TEST(memdump, md_ShouldReturnAligned16Bytes_WhenLength16Given) {
	char fixed_mem[128];
	sprintf(fixed_mem, "%16p:  14 15 16 17 18 19 1a 1b  "
			"1c 1d 1e 1f 20 21 22 23    "
			"............ !\"#", (uintptr_t *)memsrc);
	const char *argv[] = { "md", addr, "16", };
	LONGS_EQUAL(CLI_CMD_SUCCESS, cli_cmd_md.func(3, argv, &cli));
	STRNCMP_EQUAL(fixed_mem, write_spy_buffer, strlen(fixed_mem));
}

TEST(memdump, md_ShouldReturnPreviosMemoryAddr_WhenNoArgsGiven) {
	char fixed_mem[128];
	sprintf(fixed_mem, "%16p:", (uintptr_t *)memsrc);
	const char *argv[] = { "md", addr, "16", };
	cli_cmd_md.func(3, argv, &cli);
	clear_spy_buffer();
	const char *argv2[] = { "md", };
	cli_cmd_md.func(1, argv2, &cli);
	STRNCMP_EQUAL(fixed_mem, write_spy_buffer, strlen(fixed_mem));
}

TEST(memdump, md_ShouldReturnPreviousLength_WhenNoLengthGiven) {
	char fixed_mem[128];
	sprintf(fixed_mem, "%16p:  14 15 16 17 18 19 1a 1b"
			"                         ", (uintptr_t *)memsrc);
	const char *argv[] = { "md", addr, "8", };
	cli_cmd_md.func(3, argv, &cli);
	clear_spy_buffer();
	const char *argv2[] = { "md", addr, };
	cli_cmd_md.func(2, argv2, &cli);
	STRNCMP_EQUAL(fixed_mem, write_spy_buffer, strlen(fixed_mem));
}
