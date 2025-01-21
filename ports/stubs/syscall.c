/*
 * SPDX-FileCopyrightText: 2023 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/syscall.h"
#include "libmcu/compiler.h"
#include <errno.h>
#include <stdbool.h>

static syscall_writer_t syscall_write;
static syscall_reader_t syscall_read;

static bool file_exist(int file)
{
	if ((file == 1) || (file == 2) || (file == 3)) {
		return true;
	}

	return false;
}

int _close(int file)
{
	if (!file_exist(file)) {
		return -ENOENT;
	}

	return -EIO;
}

int _lseek(int file, int ptr, int dir)
{
	unused(ptr);
	unused(dir);

	if (!file_exist(file)) {
		return -ENOENT;
	}

	return -EIO;
}

int _fstat(int file, struct stat *st)
{
	unused(st);

	if (!file_exist(file)) {
		return -ENOENT;
	}

	return -EIO;
}

int _isatty(int file)
{
	if (!file_exist(file)) {
		return -ENOENT;
	}

	return -EIO;
}

int _write(int file, char *ptr, int len)
{
	if (!file_exist(file)) {
		return -ENOENT;
	}
	if (!syscall_write) {
		return 0;
	}

	return (*syscall_write)(ptr, (size_t)len);
}

int _read(int file, char *ptr, int len)
{
	if (file != 0) {
		return -ENOENT;
	}
	if (!syscall_read) {
		return 0;
	}

	return (*syscall_read)(ptr, (size_t)len);
}

int _getpid(void)
{
	return -ENOTSUP;
}

void _kill(int pid, int sig)
{
	unused(pid);
	unused(sig);
}

void syscall_register_writer(syscall_writer_t writer)
{
	syscall_write = writer;
}

void syscall_register_reader(syscall_reader_t reader)
{
	syscall_read = reader;
}
