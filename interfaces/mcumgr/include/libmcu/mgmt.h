/*
 * SPDX-FileCopyrightText: 2026 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_MGMT_H
#define LIBMCU_MGMT_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>

/**
 * @brief Management layer configuration.
 *
 * The caller supplies a raw memory pool sliced into @p buf_count buffers of
 * @p buf_size bytes each.  The transport, image, and OS backends are wired up
 * internally by the platform port (mgmt_init).
 *
 * Minimum @p buf_size: MGMT_MAX_MTU + MGMT_HDR_SIZE
 * Minimum @p buf_count: 2  (one request + one response in flight)
 */
struct mgmt_config {
	void  *buf_pool;
	size_t buf_size;
	size_t buf_count;
};

/**
 * @brief Initialize the management layer.
 *
 * Creates the transport, registers management groups (image, OS), sets up the
 * SMP streamer, and starts receiving packets.
 *
 * @param[in] cfg  Buffer pool configuration. Must remain valid for the
 *                 lifetime of the management layer.
 * @return 0 on success, negative errno on failure.
 */
int mgmt_init(const struct mgmt_config *cfg);

/**
 * @brief Stop the management layer and release its resources.
 */
void mgmt_deinit(void);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_MGMT_H */
