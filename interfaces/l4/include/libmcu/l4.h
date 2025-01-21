/*
 * SPDX-FileCopyrightText: 2022 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_L4_H
#define LIBMCU_L4_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

enum l4_event {
	L4_EVT_UNKNOWN,
	L4_EVT_CONNECTED,
	L4_EVT_DISCONNECTED,
};

struct l4_conn_param {
	const char *endpoint;
	size_t endpoint_len;

	uint16_t port;
	uint16_t timeout_ms;

	const void *ca_cert;
	size_t ca_cert_len;
	const void *client_cert;
	size_t client_cert_len;
	const void *client_key;
	size_t client_key_len;
};

struct l4;

#define l4_create_default	tls_create
#define l4_destroy_default	tls_destroy

struct l4 *tls_create(const struct l4_conn_param *param);
void tls_destroy(struct l4 *self);

int l4_connect(struct l4 *self);
int l4_disconnect(struct l4 *self);
int l4_write(struct l4 *self, const void *data, size_t data_len);
int l4_read(struct l4 *self, void *buf, size_t bufsize);

#define l4_set_ca_cert(p_conf, p_ca, l)		\
		((p_conf)->ca_cert = (p_ca), (p_conf)->ca_cert_len = (l))
#define l4_set_client_cert(p_conf, p_cert, l)	\
		((p_conf)->client_cert = (p_cert),	\
			(p_conf)->client_cert_len = (l))
#define l4_set_client_key(p_conf, p_key, l)	\
		((p_conf)->client_key = (p_key), (p_conf)->client_key_len = (l))
#define l4_set_endpoint(p_conf, p_url, l, p)	\
		((p_conf)->endpoint = (p_url), (p_conf)->endpoint_len = (l), \
			(p_conf)->port = (p))

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_L4_H */
