/*
 * SPDX-FileCopyrightText: 2022 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/l4.h"
#include "libmcu/port/l4.h"

struct l4 *tls_create(const struct l4_conn_param *param)
{
	return tls_port_create(param);
}

void tls_destroy(struct l4 *self)
{
	tls_port_destroy(self);
}

int l4_connect(struct l4 *self)
{
	return l4_port_connect(self);
}

int l4_disconnect(struct l4 *self)
{
	return l4_port_disconnect(self);
}

int l4_write(struct l4 *self, const void *data, size_t data_len)
{
	return l4_port_write(self, data, data_len);
}

int l4_read(struct l4 *self, void *buf, size_t bufsize)
{
	return l4_port_read(self, buf, bufsize);
}
