#include "commands.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "libmcu/shell.h"

#define BUFSIZE				32
#define BYTES_PER_LINE			16

#if !defined(MIN)
#define MIN(x, y)			(((x) > (y))? (y) : (x))
#endif

static void print_addr(const shell_io_t *io, uintptr_t addr)
{
	char buf[BUFSIZE] = { 0, };
	snprintf(buf, BUFSIZE-1, "%16lx:", addr);
	io->write(buf, strnlen(buf, BUFSIZE));
}

static void print_space(const shell_io_t *io, int n)
{
	for (int i = 0; i < n; i++) {
		io->write(" ", 1);
	}
}

static void print_hex(const shell_io_t *io, uint8_t val)
{
	char buf[BUFSIZE] = { 0, };
	snprintf(buf, BUFSIZE-1, " %02x", val);
	io->write(buf, strnlen(buf, BUFSIZE));
}

static void print_ascii(const shell_io_t *io, uint8_t val)
{
	char buf[BUFSIZE] = { 0, };

	if (val >= 0x20 && val < 0x7F) {
		snprintf(buf, BUFSIZE-1, "%c", val);
		io->write(buf, strnlen(buf, BUFSIZE));
	} else {
		io->write(".", 1);
	}
}

static void print_next_line(const shell_io_t *io)
{
	io->write("\r\n", 2);
}

static void memdump_hex(uintptr_t addr, int len, int width, const shell_io_t *io)
{
	print_addr(io, addr);

	const uint8_t *p = (uint8_t *)addr;

	for (int i = 0; i < width; i++) {
		if ((i % 8) == 0) {
			print_space(io, 1);
		}

		if (i >= len) {
			print_space(io, 3);
		} else {
			print_hex(io, p[i]);
		}
	}
}

static void memdump_ascii(uintptr_t addr, int len, const shell_io_t *io)
{
	const uint8_t *p = (uint8_t *)addr;

	for (int j = 0; j < len; j++) {
		print_ascii(io, p[j]);
	}
}

static void memdump(uintptr_t addr, int len, int width, const shell_io_t *io)
{
	uintptr_t start_addr = addr;
	uintptr_t end_addr = addr + (uintptr_t)len;

	while (start_addr < end_addr) {
		memdump_hex(start_addr, (int)(end_addr - start_addr), width, io);
		print_space(io, 4);
		memdump_ascii(start_addr,
				MIN(width, (int)(end_addr - start_addr)), io);
		start_addr += BYTES_PER_LINE;
		print_next_line(io);
	}
}

shell_cmd_error_t shell_cmd_memdump(int argc, const char *argv[], const void *env)
{
	static uintptr_t addr = (uintptr_t)&shell_cmd_memdump;
	static int length = BYTES_PER_LINE;

	const shell_io_t *io = env;

	switch (argc) {
	case 1: // use the same address as before
#if 0 // continued from the previous
		addr += (uintptr_t)length;
#endif
		break;
	case 2: // with cached length
		addr = (uintptr_t)strtol(argv[1], NULL, 16);
		break;
	case 3:
		addr = (uintptr_t)strtol(argv[1], NULL, 16);
		length = (int)strtol(argv[2], NULL, 10);
		break;
	default:
		break;
	}

	memdump(addr, length, BYTES_PER_LINE, io);

	return SHELL_CMD_SUCCESS;
}
