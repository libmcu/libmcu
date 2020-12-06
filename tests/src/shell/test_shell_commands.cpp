#include "CppUTest/TestHarness.h"
#include <string.h>
#include <stdlib.h>
#include "libmcu/shell.h"
#include "commands/commands.h"
#include "libmcu/system.h"

const shell_cmd_t *shell_get_command_list(void) {
	return NULL;
}

const char *system_get_version_string(void) {
	return "version\n";
}
const char *system_get_build_date_string(void) {
	return "build date\n";
}
const char *system_get_serial_number_string(void) {
	return "serial number\n";
}

static char write_spy_buffer[1024];
static size_t write_spy_buffer_index;
static size_t write_spy(const void *data, size_t data_size) {
	memcpy(write_spy_buffer + write_spy_buffer_index, data, data_size);
	write_spy_buffer_index += data_size;
	write_spy_buffer[write_spy_buffer_index] = '\0';
	return data_size;
}
static shell_io_t io = {
	.read = NULL,
	.write = write_spy,
};

TEST_GROUP(shell_commands) {
	void setup(void) {
		write_spy_buffer_index = 0;
	}
	void teardown() {
	}
};

TEST(shell_commands, exit_ShouldReturnShellCmdExit) {
	LONGS_EQUAL(SHELL_CMD_EXIT, shell_cmd_exit(1, NULL, NULL));
}

TEST(shell_commands, info_ShouldReturnAllInfo_WhenNoArgsGiven) {
	LONGS_EQUAL(SHELL_CMD_SUCCESS, shell_cmd_info(1, NULL, &io));
	STRCMP_EQUAL("version\nserial number\nbuild date\n", write_spy_buffer);
}

TEST(shell_commands, info_ShouldReturnInvalidParam_WhenMoreThan2ArgsGiven) {
	LONGS_EQUAL(SHELL_CMD_INVALID_PARAM, shell_cmd_info(3, NULL, &io));
}

TEST(shell_commands, info_ShouldReturnVersion_WhenSecondArgumentGivenAsVersion) {
	const char *argv[] = { "info", "version", };
	LONGS_EQUAL(SHELL_CMD_SUCCESS, shell_cmd_info(2, argv, &io));
	STRCMP_EQUAL("version\n", write_spy_buffer);
}

TEST(shell_commands, info_ShouldReturnSerialNumber_WhenSecondArgumentGivenAsSn) {
	const char *argv[] = { "info", "sn", };
	LONGS_EQUAL(SHELL_CMD_SUCCESS, shell_cmd_info(2, argv, &io));
	STRCMP_EQUAL("serial number\n", write_spy_buffer);
}

TEST(shell_commands, info_ShouldReturnBuildDate_WhenSecondArgumentGiven) {
	const char *argv[] = { "info", "build", };
	LONGS_EQUAL(SHELL_CMD_SUCCESS, shell_cmd_info(2, argv, &io));
	STRCMP_EQUAL("build date\n", write_spy_buffer);
}

TEST_GROUP(memdump) {
	uint8_t memsrc[1024];
	char addr[32];

	void setup(void) {
		write_spy_buffer_index = 0;
		for (int i = 0; i < (int)sizeof(memsrc); i++) {
			memsrc[i] = (uint8_t)i + 20;
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
	sprintf(fixed_mem, "%16lx:  14                    ", (uintptr_t)memsrc);
	const char *argv[] = { "md", addr, "1", };
	LONGS_EQUAL(SHELL_CMD_SUCCESS, shell_cmd_memdump(3, argv, &io));
	STRNCMP_EQUAL(fixed_mem, write_spy_buffer, strlen(fixed_mem));
}

TEST(memdump, md_ShouldReturnAligned16Bytes_WhenLength16Given) {
	char fixed_mem[128];
	sprintf(fixed_mem, "%16lx:  14 15 16 17 18 19 1a 1b  "
			"1c 1d 1e 1f 20 21 22 23    "
			"............ !\"#", (uintptr_t)memsrc);
	const char *argv[] = { "md", addr, "16", };
	LONGS_EQUAL(SHELL_CMD_SUCCESS, shell_cmd_memdump(3, argv, &io));
	STRNCMP_EQUAL(fixed_mem, write_spy_buffer, strlen(fixed_mem));
}

TEST(memdump, md_ShouldReturnPreviosMemoryAddr_WhenNoArgsGiven) {
	char fixed_mem[128];
	sprintf(fixed_mem, "%16lx:", (uintptr_t)memsrc);
	const char *argv[] = { "md", addr, "16", };
	shell_cmd_memdump(3, argv, &io);
	clear_spy_buffer();
	const char *argv2[] = { "md", };
	shell_cmd_memdump(1, argv2, &io);
	STRNCMP_EQUAL(fixed_mem, write_spy_buffer, strlen(fixed_mem));
}

TEST(memdump, md_ShouldReturnPreviousLength_WhenNoLengthGiven) {
	char fixed_mem[128];
	sprintf(fixed_mem, "%16lx:  14 15 16 17 18 19 1a 1b"
			"                         ", (uintptr_t)memsrc);
	const char *argv[] = { "md", addr, "8", };
	shell_cmd_memdump(3, argv, &io);
	clear_spy_buffer();
	const char *argv2[] = { "md", addr, };
	shell_cmd_memdump(2, argv2, &io);
	STRNCMP_EQUAL(fixed_mem, write_spy_buffer, strlen(fixed_mem));
}
