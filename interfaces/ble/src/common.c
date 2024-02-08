/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "pble/ble.h"
#include <string.h>
#include <errno.h>

void ble_adv_payload_init(struct ble_adv_payload *buf)
{
	memset(buf, 0, sizeof(*buf));
}

int ble_adv_payload_add(struct ble_adv_payload *buf, uint8_t type,
			const void *data, uint8_t data_len)
{
	if (((size_t)buf->index + data_len + 2/*len+type*/) >
			sizeof(buf->payload)) {
		return -ENOSPC;
	}

	uint8_t *p = &buf->payload[buf->index];
	buf->index = (uint8_t)(buf->index + data_len + 2);

	p[0] = (uint8_t)(data_len + 1/*type*/);
	p[1] = type;
	memcpy(&p[2], data, data_len);

	return 0;
}
