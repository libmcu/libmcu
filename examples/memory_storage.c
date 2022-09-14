/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "memory_storage.h"
#include <pthread.h>
#include <string.h>
#include "libmcu/ringbuf.h"
#include "libmcu/assert.h"

static size_t memory_peek(void *buf, size_t bufsize);
static size_t memory_read(void *buf, size_t bufsize);
static size_t memory_consume(size_t size);
static size_t memory_write(const void *data, size_t size);
static size_t memory_count(void);

static struct {
	logging_storage_t ops;
	pthread_mutex_t storage_lock;
	ringbuf_static_t storage;
	size_t count; // number of entries in the storage
} memory_storage = {
	.ops = {
		.peek = memory_peek,
		.read = memory_read,
		.consume = memory_consume,
		.write = memory_write,
		.count = memory_count,
	},
};

static size_t peek_internal(void *buf, size_t bufsize)
{
	size_t bytes_read = 0;
	size_t data_size;

	if (ringbuf_peek(&memory_storage.storage, 0, &data_size, sizeof(data_size))
			&& data_size <= bufsize) {
		bytes_read = ringbuf_peek(&memory_storage.storage, sizeof(data_size), buf, data_size);
	}

	return bytes_read;
}

static size_t memory_peek(void *buf, size_t bufsize)
{
	pthread_mutex_lock(&memory_storage.storage_lock);
	size_t bytes_read = peek_internal(buf, bufsize);
	pthread_mutex_unlock(&memory_storage.storage_lock);

	return bytes_read;
}

static size_t memory_read(void *buf, size_t bufsize)
{
	pthread_mutex_lock(&memory_storage.storage_lock);
	size_t bytes_read = peek_internal(buf, bufsize);
	if (bytes_read > 0) {
		ringbuf_consume(&memory_storage.storage,
				bytes_read + sizeof(bytes_read));
		memory_storage.count--;
	}
	pthread_mutex_unlock(&memory_storage.storage_lock);

	return bytes_read;
}

static size_t memory_consume(size_t size)
{
	size_t data_size = 0;

	pthread_mutex_lock(&memory_storage.storage_lock);
	if (ringbuf_peek(&memory_storage.storage, 0, &data_size, sizeof(data_size))) {
		if (ringbuf_consume(&memory_storage.storage, data_size + sizeof(data_size))) {
			assert(data_size == size);
			memory_storage.count--;
		} else {
			data_size = 0;
		}
	}
	pthread_mutex_unlock(&memory_storage.storage_lock);

	return data_size;
}

static size_t memory_write(const void *data, size_t size)
{
	size_t written = 0;

	pthread_mutex_lock(&memory_storage.storage_lock);
	if (ringbuf_write(&memory_storage.storage, &size, sizeof(size))) {
		written = ringbuf_write(&memory_storage.storage, data, size);
		if (written == size) {
			memory_storage.count++;
		} else {
			ringbuf_write_cancel(&memory_storage.storage, sizeof(size));
		}
	}
	pthread_mutex_unlock(&memory_storage.storage_lock);

	memory_storage_write_hook(data, size);

	return written;
}

static size_t memory_count(void)
{
	pthread_mutex_lock(&memory_storage.storage_lock);
	size_t count = memory_storage.count;
	pthread_mutex_unlock(&memory_storage.storage_lock);
	return count;
}

const logging_storage_t * __attribute__((weak))
memory_storage_init(void *storage, size_t storage_size)
{
	ringbuf_create_static(&memory_storage.storage, storage, storage_size);

	pthread_mutex_init(&memory_storage.storage_lock, NULL);
	memory_storage.count = 0;

	return &memory_storage.ops;
}

void __attribute__((weak)) memory_storage_deinit(void)
{
	pthread_mutex_destroy(&memory_storage.storage_lock);
}

#include "libmcu/compiler.h"
void LIBMCU_WEAK memory_storage_write_hook(const void *data, size_t size)
{
	unused(data);
	unused(size);
}
