/*
 * SPDX-FileCopyrightText: 2022 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/port/l4.h"

#include <errno.h>
#include <string.h>

#include "esp_tls.h"

enum tls_state {
	TLS_STATE_UNKNOWN,
	TLS_STATE_CONNECTING,
	TLS_STATE_CONNECTED,
	TLS_STATE_DISCONNECTED,
};

struct l4 {
	struct l4_conn_param param;
	enum tls_state state;
	esp_tls_t *ctx;
};

static bool is_connected(struct l4 *tls)
{
	return tls->state == TLS_STATE_CONNECTED;
}

static bool is_disconnected(struct l4 *tls)
{
	return tls->state == TLS_STATE_DISCONNECTED;
}

static void disconnect_internal(struct l4 *tls)
{
	tls->state = TLS_STATE_DISCONNECTED;
	esp_tls_conn_destroy(tls->ctx);

	tls->ctx = NULL;
}

static int connect_internal(struct l4 *tls)
{
	int rc = -ENOMEM;

	tls->state = TLS_STATE_CONNECTING;

	if ((tls->ctx = esp_tls_init()) == NULL) {
		goto out_err;
	}

	const esp_tls_cfg_t conf = {
		.cacert_buf = tls->param.ca_cert,
		.cacert_bytes = tls->param.ca_cert_len,
		.clientcert_buf = tls->param.client_cert,
		.clientcert_bytes = tls->param.client_cert_len,
		.clientkey_buf = tls->param.client_key,
		.clientkey_bytes = tls->param.client_key_len,
		.timeout_ms = tls->param.timeout_ms,
		.non_block = true,
	};

	rc = (int)esp_tls_conn_new_sync(tls->param.endpoint,
			tls->param.endpoint_len, tls->param.port, &conf, tls->ctx);

	if (rc == 1) {
		tls->state = TLS_STATE_CONNECTED;
		return 0;
	}

out_err:
	disconnect_internal(tls);
	return rc;
}

int l4_port_write(struct l4 *self, const void *data, size_t data_len)
{
	const uint8_t *p = (const uint8_t *)data;
	int bytes_sent = 0;
	int rc = 0;

	if (!is_connected(self)) {
		return -ENOTCONN;
	}

	while (bytes_sent < data_len) {
		rc = (int)esp_tls_conn_write(self->ctx, &p[bytes_sent],
			       data_len - bytes_sent);

		if (rc == ESP_TLS_ERR_SSL_WANT_READ ||
				rc == ESP_TLS_ERR_SSL_WANT_WRITE) {
			rc = 0;
		} else if (rc < 0) {
			break;
		}

		bytes_sent += rc;
	}

	return rc;
}

int l4_port_read(struct l4 *self, void *buf, size_t bufsize)
{
	if (!is_connected(self)) {
		return -ENOTCONN;
	}

	int rc = (int)esp_tls_conn_read(self->ctx, buf, bufsize);

	if (rc == ESP_TLS_ERR_SSL_WANT_READ ||
				rc == ESP_TLS_ERR_SSL_WANT_WRITE) {
		rc = 0;
	}

	return rc;
}

int l4_port_connect(struct l4 *self)
{
	if (is_connected(self)) {
		return -EISCONN;
	}

	return connect_internal(self);
}

int l4_port_disconnect(struct l4 *self)
{
	if (is_disconnected(self)) {
		return -ENOTCONN;
	}

	disconnect_internal(iface);

	return 0;
}

struct l4 *tls_port_create(const struct l4_conn_param *param)
{
	struct l4 *iface = (struct l4 *)calloc(1, sizeof(*iface));

	if (iface == NULL) {
		return NULL;
	}

	memcpy(&iface->param, param, sizeof(*param));

	return iface;
}

void tls_port_delete(struct l4 *self)
{
	free(self);
}
