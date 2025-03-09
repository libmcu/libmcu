/*
 * SPDX-FileCopyrightText: 2025 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_VERSION_H
#define LIBMCU_VERSION_H

#if defined(__cplusplus)
extern "C" {
#endif

#if !defined(MAKE_VERSION)
#define MAKE_VERSION(major, minor, patch)	\
	(((major) << 16) | ((minor) << 8) | (patch))
#endif

#define LIBMCU_VERSION_MAJOR	0
#define LIBMCU_VERSION_MINOR	2
#define LIBMCU_VERSION_BUILD	6
#define LIBMCU_VERSION		MAKE_VERSION(LIBMCU_VERSION_MAJOR, \
					LIBMCU_VERSION_MINOR, \
					LIBMCU_VERSION_BUILD)

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_VERSION_H */
