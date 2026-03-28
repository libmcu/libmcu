/*
 * SPDX-FileCopyrightText: 2026 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file mgmt_esp.c
 * @brief ESP-IDF implementation of mgmt_init() / mgmt_deinit().
 *
 * Responsibilities:
 *   - Validate the mgmt_config (buffer pool).
 *   - Set up the SMP streamer (buffer pool + raw buffer callbacks).
 *   - Register management groups (image, OS).
 *   - Start the transport.
 *
 * All platform-specific wiring (transport, image, OS) is handled internally;
 * callers only supply the buffer pool via struct mgmt_config.
 */

#include "libmcu/mgmt.h"
#include "esp_smp_transport.h"

#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include "mgmt/mgmt.h"
#include "mgmt/endian.h"
#include "smp/smp.h"
#include "img_mgmt/img_mgmt.h"
#include "os_mgmt/os_mgmt.h"

/* ---------------------------------------------------------------------------
 * Buffer pool
 * ------------------------------------------------------------------------ */

#define MGMT_BUF_COUNT_MAX  4U

struct mgmt_buf {
	uint8_t *data;
	size_t   len;
	bool     in_use;
};

struct mgmt_ctx {
	struct {
		struct mgmt_buf bufs[MGMT_BUF_COUNT_MAX];
		size_t          count;
		size_t          buf_size;
	} pool;
	struct mgmt_buf    *rsp_buf;
	struct smp_streamer streamer;
};

static struct mgmt_ctx s_ctx;

static struct mgmt_buf *pool_alloc(struct mgmt_ctx *ctx)
{
	for (size_t i = 0; i < ctx->pool.count; i++) {
		if (!ctx->pool.bufs[i].in_use) {
			ctx->pool.bufs[i].in_use = true;
			return &ctx->pool.bufs[i];
		}
	}
	return NULL;
}

static void pool_free(struct mgmt_ctx *ctx, struct mgmt_buf *buf)
{
	(void)ctx;
	if (buf) {
		buf->in_use = false;
	}
}

/* ---------------------------------------------------------------------------
 * SMP streamer callbacks
 * ------------------------------------------------------------------------ */

static void *streamer_alloc_rsp(const void *src_buf, void *arg)
{
	(void)src_buf;
	return pool_alloc((struct mgmt_ctx *)arg);
}

static void streamer_trim_front(void *buf, size_t len, void *arg)
{
	(void)arg;
	struct mgmt_buf *b = (struct mgmt_buf *)buf;
	if (!b) {
		return;
	}
	if (len >= b->len) {
		b->len = 0;
	} else {
		memmove(b->data, b->data + len, b->len - len);
		b->len -= len;
	}
}

static void streamer_reset_buf(void *buf, void *arg)
{
	(void)arg;
	struct mgmt_buf *b = (struct mgmt_buf *)buf;
	if (b) {
		b->len = 0;
	}
}

static int streamer_write_at(void *tx_buf,
		size_t offset, const void *data, size_t len, void *arg)
{
	struct mgmt_ctx *ctx = (struct mgmt_ctx *)arg;
	struct mgmt_buf *b = ctx->rsp_buf;
	if (!b || offset + len > ctx->pool.buf_size) {
		return MGMT_ERR_ENOMEM;
	}

	memcpy((uint8_t *)tx_buf + offset, data, len);
	if (offset + len > b->len) {
		b->len = offset + len;
	}

	return 0;
}

static int streamer_init_reader(void *buf, void *arg,
		const void **out_rx_buf, size_t *out_rx_len)
{
	(void)arg;

	struct mgmt_buf *b = (struct mgmt_buf *)buf;
	if (!b) {
		return MGMT_ERR_EINVAL;
	}

	*out_rx_buf = b->data;
	*out_rx_len = b->len;
	return 0;
}

static int streamer_init_writer(void *buf, void *arg,
		void **out_tx_buf, size_t *out_tx_size)
{
	struct mgmt_ctx *ctx = (struct mgmt_ctx *)arg;
	struct mgmt_buf *b = (struct mgmt_buf *)buf;
	if (!b) {
		return MGMT_ERR_EINVAL;
	}

	ctx->rsp_buf = b;
	b->len = 0;

	*out_tx_buf  = b->data;
	*out_tx_size = ctx->pool.buf_size;
	return 0;
}

static void streamer_free_buf(void *buf, void *arg)
{
	struct mgmt_ctx *ctx = (struct mgmt_ctx *)arg;
	struct mgmt_buf *b = (struct mgmt_buf *)buf;
	if (b) {
		if (ctx->rsp_buf == b) {
			ctx->rsp_buf = NULL;
		}
		pool_free(ctx, b);
	}
}

static const struct mgmt_streamer_cfg s_streamer_cfg = {
	.alloc_rsp   = streamer_alloc_rsp,
	.trim_front  = streamer_trim_front,
	.reset_buf   = streamer_reset_buf,
	.write_at    = streamer_write_at,
	.init_reader = streamer_init_reader,
	.init_writer = streamer_init_writer,
	.free_buf    = streamer_free_buf,
};

/* ---------------------------------------------------------------------------
 * SMP TX callback — called by smp_process_request_packet() with the response.
 * ------------------------------------------------------------------------ */

static int smp_tx_rsp(struct smp_streamer *ss, void *buf, void *arg)
{
	struct mgmt_ctx *ctx = (struct mgmt_ctx *)arg;
	struct mgmt_buf *b = (struct mgmt_buf *)buf;
	if (!b) {
		return MGMT_ERR_EINVAL;
	}

	/*
	 * The CBOR encoder writes directly to b->data + MGMT_HDR_SIZE via its
	 * internal buf_writer, bypassing write_at.  b->len therefore only
	 * reflects the 8-byte header written through write_at.  Compute the
	 * true total length from the SMP header's nh_len field (network order).
	 */
	struct mgmt_hdr hdr;
	memcpy(&hdr, b->data, sizeof(hdr));
	size_t total_len = MGMT_HDR_SIZE + ntohs(hdr.nh_len);

	int rc = 0;
	if (total_len > MGMT_HDR_SIZE && total_len <= ctx->pool.buf_size) {
		rc = esp_smp_transport_send(b->data, total_len);
		if (rc != 0) {
			rc = MGMT_ERR_EUNKNOWN;
		}
	}

	mgmt_streamer_free_buf(&ss->mgmt_stmr, b);
	return rc;
}

/* ---------------------------------------------------------------------------
 * RX callback — called by the transport for each decoded SMP packet.
 * ------------------------------------------------------------------------ */

static void on_packet_received(const void *data, size_t len, void *user_ctx)
{
	struct mgmt_ctx *ctx = (struct mgmt_ctx *)user_ctx;

	if (!data || len == 0 || len > ctx->pool.buf_size) {
		return;
	}

	struct mgmt_buf *req = pool_alloc(ctx);
	if (!req) {
		return;
	}

	memcpy(req->data, data, len);
	req->len = len;

	smp_process_request_packet(&ctx->streamer, req);
}

int mgmt_init(const struct mgmt_config *cfg)
{
	if (!cfg || !cfg->buf_pool || cfg->buf_size == 0
			|| cfg->buf_count == 0
			|| cfg->buf_count > MGMT_BUF_COUNT_MAX) {
		return -EINVAL;
	}

	/* Slice the caller-provided pool into individual buffers. */
	s_ctx.pool.count    = cfg->buf_count;
	s_ctx.pool.buf_size = cfg->buf_size;
	for (size_t i = 0; i < cfg->buf_count; i++) {
		s_ctx.pool.bufs[i].data   = &((uint8_t *)cfg->buf_pool)[i * cfg->buf_size];
		s_ctx.pool.bufs[i].len    = 0;
		s_ctx.pool.bufs[i].in_use = false;
	}

	/* Wire up the SMP streamer. */
	s_ctx.streamer.mgmt_stmr.cfg    = &s_streamer_cfg;
	s_ctx.streamer.mgmt_stmr.cb_arg = &s_ctx;
	s_ctx.streamer.tx_rsp_cb        = smp_tx_rsp;

	img_mgmt_register_group();
	os_mgmt_register_group();

	return esp_smp_transport_init(on_packet_received, &s_ctx);
}

void mgmt_deinit(void)
{
	esp_smp_transport_deinit();
}
