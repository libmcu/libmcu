// maximum key and namespace string length is 15 bytes
#include "nvs_kvstore.h"

#include <stdbool.h>
#include <stddef.h>
#include <assert.h>

#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"

struct nvs_kvstore_s {
	kvstore_t ops;
	nvs_handle_t handle;
};

static int nvs_kvstore_open(const char *ns, nvs_handle_t *namespace_handle)
{
	return !nvs_open(ns, NVS_READWRITE, namespace_handle);
}

static size_t nvs_kvstore_write(kvstore_t *kvstore,
		const char *key, const void *value, size_t size)
{
	struct nvs_kvstore_s *p = (typeof(p))kvstore;

	if (nvs_set_blob(p->handle, key, value, size)) {
		return 0;
	}

	return !nvs_commit(p->handle)? size : 0;
}

static size_t nvs_kvstore_read(const kvstore_t *kvstore,
		const char *key, void *buf, size_t bufsize)
{
	const struct nvs_kvstore_s *p = (typeof(p))kvstore;
	return !nvs_get_blob(p->handle, key, buf, &bufsize)? bufsize : 0;
}

kvstore_t *nvs_kvstore_new(const char *ns)
{
	struct nvs_kvstore_s *p;

	if (!(p = malloc(sizeof(*p)))) {
		return NULL;
	}

	if (!nvs_kvstore_open(ns, &p->handle)) {
		free(p);
		return NULL;
	}

	p->ops = (typeof(p->ops)) {
		.write = nvs_kvstore_write,
		.read = nvs_kvstore_read,
	};

	return &p->ops;
}

void nvs_kvstore_delete(kvstore_t *kvstore)
{
	nvs_close(((typeof(struct nvs_kvstore_s *))kvstore)->handle);
	free(kvstore);
}

int nvs_kvstore_init(void)
{
	return nvs_flash_init();
}
