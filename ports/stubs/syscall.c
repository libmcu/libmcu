/*
 * SPDX-FileCopyrightText: 2023 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/syscall.h"
#include "libmcu/compiler.h"
#include <errno.h>

static syscall_writer_t syscall_write;
static syscall_reader_t syscall_read;

int _close(int file)
{
	unused(file);
	return -EIO;
}

int _lseek(int file, int ptr, int dir)
{
	unused(file);
	unused(ptr);
	unused(dir);
	return -EIO;
}

int _fstat(int file, struct stat *st)
{
	unused(file);
	unused(st);
	return -EIO;
}

int _isatty(int file)
{
	unused(file);
	return -EIO;
}

int _write(int file, char *ptr, int len)
{
	if ((file != 1) && (file != 2) && (file != 3)) {
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

void syscall_register_writer(syscall_writer_t writer)
{
	syscall_write = writer;
}

void syscall_register_reader(syscall_reader_t reader)
{
	syscall_read = reader;
}
