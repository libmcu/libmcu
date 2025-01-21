/*
 * SPDX-FileCopyrightText: 2022 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_L4_PORT_H
#define LIBMCU_L4_PORT_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "libmcu/l4.h"

struct l4 *tls_port_create(const struct l4_conn_param *param);
void tls_port_destroy(struct l4 *self);

int l4_port_connect(struct l4 *self);
int l4_port_disconnect(struct l4 *self);
int l4_port_write(struct l4 *self, const void *data, size_t data_len);
int l4_port_read(struct l4 *self, void *buf, size_t bufsize);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_L4_PORT_H */
