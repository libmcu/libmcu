/*
 * SPDX-FileCopyrightText: 2020 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LOGGING_BACKEND_H
#define LOGGING_BACKEND_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>

#if !defined(LOGGING_MAX_BACKENDS)
#define LOGGING_MAX_BACKENDS			1
#endif

struct logging_backend {
	size_t (*write)(const void *data, size_t size);
	size_t (*peek)(void *buf, size_t bufsize);
	/** Read up to `bufsize` bytes from the storage
	 * @return The number of bytes read is returned on success,
	 *         0 is returned on error or when no log is there. */
	size_t (*read)(void *buf, size_t bufsize);
	/** Remove the oldest log in the storage
	 * @return The number of bytes removed. */
	size_t (*consume)(size_t size);
	size_t (*count)(void);
};

#if defined(__cplusplus)
}
#endif

#endif /* LOGGING_BACKEND_H */
