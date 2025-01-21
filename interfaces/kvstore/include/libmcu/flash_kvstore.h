/*
 * SPDX-FileCopyrightText: 2024 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef FLASH_KVSTORE_H
#define FLASH_KVSTORE_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "libmcu/kvstore.h"
#include "libmcu/flash.h"

struct kvstore *flash_kvstore_new(struct flash *flash, struct flash *scratch);

#if defined(__cplusplus)
}
#endif

#endif
