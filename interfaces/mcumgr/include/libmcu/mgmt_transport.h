/*
 * SPDX-FileCopyrightText: 2026 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_MGMT_TRANSPORT_H
#define LIBMCU_MGMT_TRANSPORT_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>

/**
 * @brief Callback invoked by the transport when a complete SMP packet arrives.
 *
 * @param[in] data  Decoded SMP packet (framing already stripped).
 * @param[in] len   Length of the packet in bytes.
 * @param[in] ctx   Opaque context supplied to the transport init function.
 */
typedef void (*mgmt_transport_rx_callback_t)(const void *data, size_t len,
		void *ctx);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_MGMT_TRANSPORT_H */
