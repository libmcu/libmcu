/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/msgq.h"

#include <stdlib.h>
#include <errno.h>

#include "libmcu/ringbuf.h"

struct msgq {
	struct ringbuf *ringbuf;
	size_t capacity;

	msgq_lock_fn lock;
	msgq_unlock_fn unlock;
	void *sync_ctx;
};

static int push_message(struct ringbuf *ringbuf,
		const void *data, const size_t datasize)
{
	const size_t available =
		ringbuf_capacity(ringbuf) - ringbuf_length(ringbuf);
	const msgq_msg_meta_t meta = {
		.size = datasize,
	};
	size_t n;

	if (datasize + sizeof(meta) > available) {
		return -ENOMEM;
	}

	if ((n = ringbuf_write(ringbuf, &meta, sizeof(meta))) != sizeof(meta)) {
		ringbuf_write_cancel(ringbuf, n);
		return -EIO;
	}

	if ((n = ringbuf_write(ringbuf, data, datasize)) != datasize) {
		ringbuf_write_cancel(ringbuf, n + sizeof(meta));
		return -EIO;
	}

	return 0;
}

static int pop_message(struct ringbuf *ringbuf, void *buf, size_t bufsize)
{
	msgq_msg_meta_t meta;

	if (ringbuf_peek(ringbuf, 0, &meta, sizeof(meta)) != sizeof(meta)) {
		return -ENOENT;
	}

	if (meta.size > bufsize) {
		return -ERANGE;
	}

	if (ringbuf_peek(ringbuf, sizeof(meta), buf, meta.size) != meta.size) {
		return -EIO;
	}

	ringbuf_consume(ringbuf, sizeof(meta) + meta.size);

	return (int)meta.size;
}

int msgq_push(struct msgq *q, const void *data, const size_t datasize)
{
	if (q->lock && q->lock(q->sync_ctx) != 0) {
		return -EAGAIN;
	}

	int err = push_message(q->ringbuf, data, datasize);

	if (q->unlock) {
		err |= q->unlock(q->sync_ctx);
	}

	return err;
}

int msgq_pop(struct msgq *q, void *buf, size_t bufsize)
{
	if (q->lock && q->lock(q->sync_ctx) != 0) {
		return -EAGAIN;
	}

	int bytes_read = pop_message(q->ringbuf, buf, bufsize);

	if (q->unlock) {
		q->unlock(q->sync_ctx);
	}

	return bytes_read;
}

size_t msgq_next_msg_size(const struct msgq *q)
{
	msgq_msg_meta_t meta;

	if (q->lock && q->lock(q->sync_ctx) != 0) {
		return 0;
	}

	if (ringbuf_peek(q->ringbuf, 0, &meta, sizeof(meta)) != sizeof(meta)) {
		return 0;
	}

	if (q->unlock) {
		q->unlock(q->sync_ctx);
	}

	return meta.size;
}

size_t msgq_available(const struct msgq *q)
{
	if (q->lock && q->lock(q->sync_ctx) != 0) {
		return 0;
	}

	const size_t available = ringbuf_capacity(q->ringbuf)
		- ringbuf_length(q->ringbuf);

	if (q->unlock) {
		q->unlock(q->sync_ctx);
	}

	return available < sizeof(msgq_msg_meta_t) ?
		0 : available - sizeof(msgq_msg_meta_t);
}

size_t msgq_cap(const struct msgq *q)
{
	if (q->lock && q->lock(q->sync_ctx) != 0) {
		return 0;
	}

	size_t cap = ringbuf_capacity(q->ringbuf);

	if (q->unlock) {
		q->unlock(q->sync_ctx);
	}

	return cap;
}

size_t msgq_len(const struct msgq *q)
{
	if (q->lock && q->lock(q->sync_ctx) != 0) {
		return 0;
	}

	size_t len = ringbuf_length(q->ringbuf);

	if (q->unlock) {
		q->unlock(q->sync_ctx);
	}

	return len;
}

int msgq_set_sync(struct msgq *q,
		msgq_lock_fn lock, msgq_unlock_fn unlock, void *ctx)
{
	q->lock = lock;
	q->unlock = unlock;
	q->sync_ctx = ctx;

	return 0;
}

size_t msgq_calc_size(const size_t n, const size_t max_msg_size)
{
	return (sizeof(msgq_msg_meta_t) + max_msg_size) * n;
}

struct msgq *msgq_create(const size_t capacity_bytes)
{
	struct msgq *q;

	if (capacity_bytes == 0) {
		return NULL;
	}

	if ((q = (struct msgq *)malloc(sizeof(struct msgq)))) {
		if ((q->ringbuf = ringbuf_create(capacity_bytes)) == NULL) {
			free(q);
			return NULL;
		}

		q->capacity = capacity_bytes;
		q->lock = NULL;
		q->unlock = NULL;
		q->sync_ctx = NULL;
	}

	return q;
}

void msgq_destroy(struct msgq *q)
{
	if (q) {
		ringbuf_destroy(q->ringbuf);
		free(q);
	}
}
