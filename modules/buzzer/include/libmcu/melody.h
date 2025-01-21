/*
 * SPDX-FileCopyrightText: 2023 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_MELODY_H
#define LIBMCU_MELODY_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "tone.h"

struct melody {
	const struct tone *tones;
	uint8_t nr_tones;
};

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_MELODY_H */
