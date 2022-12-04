/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef NVS_KVSTORE_H
#define NVS_KVSTORE_H

#include "libmcu/kvstore.h"

/**
 * @brief Create a non-volatile storage instance
 *
 * @note Initialization of the storage must be done before calling this function.
 *
 * @return an instance
 */
struct kvstore *nvs_kvstore_new(void);
int nvs_kvstore_count(void);

#endif
