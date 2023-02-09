/*
 * SPDX-FileCopyrightText: 2023 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_SYSCALL_H
#define LIBMCU_SYSCALL_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <sys/stat.h>

typedef int (*syscall_writer_t)(const void *data, size_t data_len);
typedef int (*syscall_reader_t)(void *buf, size_t bufsize);

int _close(int file);
int _lseek(int file, int ptr, int dir);
int _fstat(int file, struct stat *st);
int _isatty(int file);
int _write(int file, char *ptr, int len);
int _read(int file, char *ptr, int len);
int _getpid(void);
void _kill(int pid, int sig);

void syscall_register_writer(syscall_writer_t writer);
void syscall_register_reader(syscall_reader_t reader);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_SYSCALL_H */
