/*
 * SPDX-FileCopyrightText: 2024 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/metricfs.h"

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdbool.h>

#define PREFIX_MAXLEN		(METRICFS_ID_MAXLEN - 6) /* metricfs_id_t
					represents 5 digits + 1 for slash. */
struct metricfs {
	struct kvstore *kvstore;
	const char *prefix;
	size_t max_metrics;

	metricfs_id_t index;
	metricfs_id_t outdex;
};

struct iterator_ctx {
	metricfs_iterator_t cb;
	void *cb_ctx;
	void *buf;
	size_t bufsize;

	metricfs_id_t id;
};

typedef bool (*iterator_fn_t)(struct metricfs *fs,
		const char *keystr, struct iterator_ctx *ctx);

static int read_index(struct metricfs *fs, metricfs_id_t *index)
{
	char keystr[METRICFS_ID_MAXLEN+1];
	snprintf(keystr, sizeof(keystr)-1, "%s/index", fs->prefix);
	int err = kvstore_read(fs->kvstore, keystr, index, sizeof(*index));
	if (err == -ENOENT) {
		*index = 0;
		err = 0;
	}
	return err;
}

static int read_outdex(struct metricfs *fs, metricfs_id_t *outdex)
{
	char keystr[METRICFS_ID_MAXLEN+1];
	snprintf(keystr, sizeof(keystr)-1, "%s/outdex", fs->prefix);
	int err = kvstore_read(fs->kvstore, keystr, outdex, sizeof(*outdex));
	if (err == -ENOENT) {
		*outdex = 0;
		err = 0;
	}
	return err;
}

static int update_index(struct metricfs *fs, const metricfs_id_t index)
{
	char keystr[METRICFS_ID_MAXLEN+1];
	snprintf(keystr, sizeof(keystr)-1, "%s/index", fs->prefix);
	return kvstore_write(fs->kvstore, keystr, &index, sizeof(index));
}

static int update_outdex(struct metricfs *fs, const metricfs_id_t outdex)
{
	char keystr[METRICFS_ID_MAXLEN+1];
	snprintf(keystr, sizeof(keystr)-1, "%s/outdex", fs->prefix);
	return kvstore_write(fs->kvstore, keystr, &outdex, sizeof(outdex));
}

static uint16_t count_metrics(const struct metricfs *fs)
{
	return fs->index - fs->outdex;
}

static int peek_first(struct metricfs *fs, void *buf, const size_t bufsize)
{
	if (count_metrics(fs) == 0) {
		return -ENOENT;
	}

	char keystr[METRICFS_ID_MAXLEN+1];
	snprintf(keystr, sizeof(keystr)-1, "%s/%u", fs->prefix, fs->outdex);
	return kvstore_read(fs->kvstore, keystr, buf, bufsize);
}

static int delete_first(struct metricfs *fs)
{
	if (count_metrics(fs) == 0) {
		return -ENOENT;
	}

	char keystr[METRICFS_ID_MAXLEN+1];
	snprintf(keystr, sizeof(keystr)-1, "%s/%u", fs->prefix, fs->outdex);
	int err = kvstore_clear(fs->kvstore, keystr);

	if (err < 0 && err != -ENOENT) {
		return err;
	}

	if ((err = update_outdex(fs, fs->outdex + 1)) == sizeof(fs->outdex)) {
		fs->outdex += 1;
		return 0;
	}

	return err;
}

static bool on_read_iteration(struct metricfs *fs,
		const char *keystr, struct iterator_ctx *ctx)
{
	int err = kvstore_read(fs->kvstore, keystr, ctx->buf, ctx->bufsize);

	if (err < 0) {
		return false;
	} else if (ctx->cb) {
		(*ctx->cb)(ctx->id, ctx->buf, (size_t)err, ctx->cb_ctx);
	}

	return true;
}

static bool on_clear_iteration(struct metricfs *fs,
		const char *keystr, struct iterator_ctx *ctx)
{
	(void)ctx;
	bool success = false;

	if (kvstore_clear(fs->kvstore, keystr) >= 0) {
		success = true;
	}

	fs->outdex += 1;

	return success;
}

static int iterate(struct metricfs *fs, iterator_fn_t fn,
		struct iterator_ctx *ctx, const uint16_t max_count)
{
	const metricfs_id_t outdex = fs->outdex;
	int err = 0;

	for (uint16_t i = 0; i < max_count; i++) {
		const metricfs_id_t id = outdex + i;
		char keystr[METRICFS_ID_MAXLEN+1];

		snprintf(keystr, sizeof(keystr)-1, "%s/%u", fs->prefix, id);
		ctx->id = id;

		if (!(*fn)(fs, keystr, ctx)) {
			err = -EIO;
		}
	}

	return err;
}

int metricfs_write(struct metricfs *fs,
		const void *data, const size_t datasize, metricfs_id_t *id)
{
	char keystr[METRICFS_ID_MAXLEN+1];
	int err;

	if (count_metrics(fs) >= fs->max_metrics) {
		return -ENOSPC;
	}

	snprintf(keystr, sizeof(keystr)-1, "%s/%u", fs->prefix, fs->index);
	err = kvstore_write(fs->kvstore, keystr, data, datasize);
	if ((size_t)err != datasize) {
		return err;
	}

	if ((err = update_index(fs, fs->index + 1)) == sizeof(fs->index)) {
		if (id) {
			*id = fs->index;
		}
		fs->index += 1;

		return 0;
	}

	return err;
}

uint16_t metricfs_count(const struct metricfs *fs)
{
	return count_metrics(fs);
}

int metricfs_iterate(struct metricfs *fs,
		metricfs_iterator_t cb, void *cb_ctx,
		void *buf, const size_t bufsize, const size_t max_metrics)
{
	struct iterator_ctx ctx = {
		.cb = cb,
		.cb_ctx = cb_ctx,
		.buf = buf,
		.bufsize = bufsize,
	};
	uint16_t n = count_metrics(fs);

	if (n == 0) {
		return -ENOENT;
	}

	if (max_metrics > 0 && (uint16_t)max_metrics < n) {
		n = (uint16_t)max_metrics;
	}

	return iterate(fs, on_read_iteration, &ctx, n);
}

int metricfs_peek(struct metricfs *fs,
		const metricfs_id_t id, void *buf, const size_t bufsize)
{
	char keystr[METRICFS_ID_MAXLEN+1];
	snprintf(keystr, sizeof(keystr)-1, "%s/%u", fs->prefix, id);
	return kvstore_read(fs->kvstore, keystr, buf, bufsize);
}

int metricfs_peek_first(struct metricfs *fs,
		void *buf, const size_t bufsize, metricfs_id_t *id)
{
	int err = peek_first(fs, buf, bufsize);

	if (err > 0 && id) {
		*id = fs->outdex;
	}

	return err;
}

int metricfs_read_first(struct metricfs *fs,
		void *buf, const size_t bufsize, metricfs_id_t *id)
{
	int err = peek_first(fs, buf, bufsize);

	if (err > 0 || err == -ENOENT) {
		if (err > 0 && id) {
			*id = fs->outdex;
		}

		if (delete_first(fs) < 0) {
			err = -EIO;
		}
	}

	return err;
}

int metricfs_del_first(struct metricfs *fs, metricfs_id_t *id)
{
	if (id) {
		*id = fs->outdex;
	}

	return delete_first(fs);
}

int metricfs_clear(struct metricfs *fs)
{
	struct iterator_ctx ctx = {
		.buf = NULL,
		.bufsize = 0,
	};
	const uint16_t n = count_metrics(fs);

	if (n == 0) {
		return -ENOENT;
	}

	int err = iterate(fs, on_clear_iteration, &ctx, n);
	err |= update_outdex(fs, fs->outdex);

	return (err >= 0)? 0 : err;
}

struct metricfs *metricfs_create(struct kvstore *kvstore,
		const char *prefix, const size_t max_metrics)
{
	static struct metricfs fs;

	if (!kvstore || !prefix || max_metrics == 0 ||
			strlen(prefix) > PREFIX_MAXLEN) {
		return NULL;
	}

	fs = (struct metricfs) {
		.kvstore = kvstore,
		.prefix = prefix,
		.max_metrics = max_metrics,
		.index = 0,
		.outdex = 0,
	};

	if (read_index(&fs, &fs.index) < 0 ||
			read_outdex(&fs, &fs.outdex) < 0) {
		return NULL;
	}

	return &fs;
}

void metricfs_destroy(struct metricfs *fs)
{
	memset(fs, 0, sizeof(*fs));
}
