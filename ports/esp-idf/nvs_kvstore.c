/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

// maximum key and namespace string length is 15 bytes
#include "libmcu/nvs_kvstore.h"

#include <stdbool.h>
#include <stddef.h>

#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "libmcu/assert.h"

struct kvstore {
	struct kvstore_api api;
	nvs_handle_t handle;
};

static int nvs_storage_open(char const *namespace, nvs_handle_t *namespace_handle)
{
	return nvs_open(namespace, NVS_READWRITE, namespace_handle);
}

static void nvs_storage_close(nvs_handle_t *namespace_handle)
{
	nvs_close(*namespace_handle);
}

static int nvs_kvstore_write(struct kvstore *self, char const *key, void const *value, size_t size)
{
	int err = nvs_set_blob(self->handle, key, value, size);
	if (err) {
		return 0;
	}
	return !nvs_commit(self->handle)? size : 0;
}

static int nvs_kvstore_read(struct kvstore *self, char const *key, void *buf, size_t size)
{
	return !nvs_get_blob(self->handle, key, buf, &size)? size : 0;
}

static int nvs_kvstore_erase(struct kvstore *self, char const *key)
{
	return nvs_erase_key(self->handle, key);
}

static int nvs_kvstore_open(struct kvstore *self, char const *namespace)
{
	return nvs_storage_open(namespace, &self->handle);
}

static void nvs_kvstore_close(struct kvstore *self)
{
	nvs_storage_close(&self->handle);
}

/* NOTE: nvs should be initialized first before calling the function. Call
 * `nvs_flash_init()` first to initialize. */
struct kvstore *nvs_kvstore_new(void)
{
	struct kvstore *nvs_kvstore = malloc(sizeof(*nvs_kvstore));
	assert(nvs_kvstore);

	nvs_kvstore->api.write = nvs_kvstore_write;
	nvs_kvstore->api.read = nvs_kvstore_read;
	nvs_kvstore->api.clear = nvs_kvstore_erase;
	nvs_kvstore->api.open = nvs_kvstore_open;
	nvs_kvstore->api.close = nvs_kvstore_close;

	return nvs_kvstore;
}
