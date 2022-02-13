#include "libmcu/nvs_kvstore.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "nrf.h"
#include "nordic_common.h"
#ifdef SOFTDEVICE_PRESENT
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#else
#include "nrf_drv_clock.h"
#endif
#include "fds.h"

#include "libmcu/logging.h"
#include "libmcu/assert.h"

struct nvs_kvstore {
	struct kvstore super;
	uint16_t file_id;
	fds_record_desc_t desc;
	fds_find_token_t tok;
};

static volatile bool initialized = false;

static void fds_evt_handler(fds_evt_t const * p_evt)
{
	if (p_evt->result != NRF_SUCCESS) {
		error("EVT: %d, %d", p_evt->id, p_evt->result);
	}

	switch (p_evt->id) {
	case FDS_EVT_INIT:
		if (p_evt->result == NRF_SUCCESS) {
			initialized = true;
		}
		break;
	case FDS_EVT_WRITE:
		if (p_evt->result == NRF_SUCCESS) {
			debug("written: %04x, %04x, %04x",
					p_evt->write.record_id,
					p_evt->write.file_id,
					p_evt->write.record_key);
		}
		break;
	case FDS_EVT_DEL_RECORD:
		if (p_evt->result == NRF_SUCCESS) {
			debug("deleted: %04x, %04x, %04x",
					p_evt->del.record_id,
					p_evt->del.file_id,
					p_evt->del.record_key);
		}
		break;
	default:
		 break;
	}
}

static int nvs_storge_init(void)
{

	if (initialized) {
		return 0;
	}

	fds_register(fds_evt_handler);
	ret_code_t rc = fds_init();
	assert(rc == NRF_SUCCESS);

	while (!initialized) {
#ifdef SOFTDEVICE_PRESENT
		(void) sd_app_evt_wait();
#else
		__WFE();
#endif
	}

	return 0;
}

static bool has_record(struct kvstore const *self, uint16_t record_key)
{
	struct nvs_kvstore *nvs_kvstore = (struct nvs_kvstore *)self;

	memset(&nvs_kvstore->desc, 0, sizeof(nvs_kvstore->desc));
	memset(&nvs_kvstore->tok, 0, sizeof(nvs_kvstore->desc));

	if (fds_record_find(nvs_kvstore->file_id, record_key, &nvs_kvstore->desc,
				&nvs_kvstore->tok) == NRF_SUCCESS) {
		return true;
	}

	return false;
}

static int nvs_kvstore_write(struct kvstore *self, char const *key, void const *value, size_t size)
{
	struct nvs_kvstore *nvs_kvstore = (struct nvs_kvstore *)self;
	uint16_t record_key = (uint16_t)strtol(key, NULL, 16);
	fds_record_t new_record = {
		.file_id = nvs_kvstore->file_id,
		.key = record_key,
		.data.p_data = value,
		.data.length_words = (size + 3) / sizeof(uint32_t),
	};
	ret_code_t rc;

	if (!has_record(self, record_key)) {
		if ((rc = fds_record_write(&nvs_kvstore->desc, &new_record))
				!= NRF_SUCCESS) {
			goto out_err;
		}

		return size;
	}

        if ((rc = fds_record_update(&nvs_kvstore->desc, &new_record))
			== NRF_SUCCESS) {
		return size;
	}

out_err:
	error("%x", rc);
	return 0;
}

static int count_records(void)
{
	fds_find_token_t tok = { 0, };
	fds_record_desc_t desc = { 0, };
	int i = 0;

	while (fds_record_iterate(&desc, &tok) == NRF_SUCCESS) {
		i++;
	}

	return i;
}

static int nvs_kvstore_read(struct kvstore *self, char const *key, void *buf, size_t size)
{
	struct nvs_kvstore *nvs_kvstore = (struct nvs_kvstore *)self;
	uint16_t record_key = (uint16_t)strtol(key, NULL, 16);
        fds_flash_record_t config = { 0, };

	if (!has_record(self, record_key)) {
		return 0;
	}
	if (fds_record_open(&nvs_kvstore->desc, &config) != NRF_SUCCESS) {
		return 0;
	}

        memcpy(buf, config.p_data, size);

        fds_record_close(&nvs_kvstore->desc);

	return size;
}

static int nvs_kvstore_open(struct kvstore *self, char const *namespace)
{
	struct nvs_kvstore *nvs_kvstore = (struct nvs_kvstore *)self;
	nvs_kvstore->file_id = (uint16_t)strtol(namespace, NULL, 16);

	return 0;
}

static void nvs_kvstore_close(struct kvstore *self)
{
	(void)self;
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

int nvs_kvstore_count(void)
{
	return count_records();
}
