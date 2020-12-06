#include "CppUTest/TestHarness.h"
#include <string.h>
#include "libmcu/shell.h"
#include "commands/commands.h"

TEST_GROUP(shell_commands) {
	void setup(void) {
	}
	void teardown() {
	}
};

TEST(shell_commands, exit_ShouldReturnShellCmdExit) {
	LONGS_EQUAL(SHELL_CMD_EXIT, g_cmd_exit.run(1, NULL, NULL));
}
