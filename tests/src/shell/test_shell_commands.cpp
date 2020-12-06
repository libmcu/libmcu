#include "CppUTest/TestHarness.h"
#include <string.h>
#include "commands/commands.h"

const shell_cmd_t *shell_get_command_list(void)
{
	return NULL;
}

TEST_GROUP(shell_commands) {
	void setup(void) {
	}
	void teardown() {
	}
};

TEST(shell_commands, exit_ShouldReturnShellCmdExit) {
	LONGS_EQUAL(SHELL_CMD_EXIT, shell_cmd_exit(1, NULL, NULL));
}
