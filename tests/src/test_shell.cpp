#include "CppUTest/TestHarness.h"
#include <string.h>
#include "libmcu/shell.h"

extern "C" {
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
}

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
		STRNCMP_EQUAL(s, readbuf, strlen(s));
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

TEST(shell, shell_ShouldReturnSuccess_WhenHelpCommandGiven) {
	given("help\nexit\n");
	shell_run(&io);
	then("$ help\r\nexit\t: Exit the shell\r\n");
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
