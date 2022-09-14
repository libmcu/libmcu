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

struct nvs_kvstore {
	struct kvstore super;
	nvs_handle_t handle;
};

static bool initialized = false;

static int nvs_storge_init(void)
{
	if (initialized) {
		return 0;
	}

	esp_err_t err = nvs_flash_init();
	switch (err) {
		case ESP_OK:
			break;
		case ESP_ERR_NVS_NO_FREE_PAGES:
		case ESP_ERR_NVS_NEW_VERSION_FOUND:
			return -1;
		default:
			return -2;
	}

	initialized = true;

	return 0;
}

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
	struct nvs_kvstore *nvs_kvstore = (struct nvs_kvstore *)self;

	int err = nvs_set_blob(nvs_kvstore->handle, key, value, size);
	if (err) {
		return 0;
	}
	return !nvs_commit(nvs_kvstore->handle)? size : 0;
}

static int nvs_kvstore_read(struct kvstore *self, char const *key, void *buf, size_t size)
{
	struct nvs_kvstore *nvs_kvstore = (struct nvs_kvstore *)self;
	return !nvs_get_blob(nvs_kvstore->handle, key, buf, &size)? size : 0;
}

static int nvs_kvstore_open(struct kvstore *self, char const *namespace)
{
	struct nvs_kvstore *nvs_kvstore = (struct nvs_kvstore *)self;
	return nvs_storage_open(namespace, &nvs_kvstore->handle);
}

static void nvs_kvstore_close(struct kvstore *self)
{
	struct nvs_kvstore *nvs_kvstore = (struct nvs_kvstore *)self;
	nvs_storage_close(&nvs_kvstore->handle);
}

struct kvstore *nvs_kvstore_new(void)
{
	struct nvs_kvstore *nvs_kvstore = malloc(sizeof(*nvs_kvstore));
	assert(nvs_kvstore);

	int rc = nvs_storge_init();
	assert(rc == 0);

	struct kvstore *kvstore = (struct kvstore *)nvs_kvstore;
	kvstore->write = nvs_kvstore_write;
	kvstore->read = nvs_kvstore_read;
	kvstore->open = nvs_kvstore_open;
	kvstore->close = nvs_kvstore_close;

	return kvstore;
}
