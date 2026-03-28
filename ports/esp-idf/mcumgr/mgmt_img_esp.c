/*
 * SPDX-FileCopyrightText: 2026 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "img_mgmt/img_mgmt.h"
#include "img_mgmt/img_mgmt_impl.h"
#include "img_mgmt/image.h"
#include "mgmt/mgmt.h"

#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_log.h"

#define TAG "mgmt_img_esp"

/* Module-level OTA state — only one upload can be active at a time. */
static struct {
	esp_ota_handle_t handle;
} s_ota;

static const esp_partition_t *slot_to_partition(int slot)
{
	if (slot == 0) {
		return esp_ota_get_running_partition();
	}
	/*
	 * After a completed upload, esp_ota_set_boot_partition() has already
	 * been called, so esp_ota_get_next_update_partition() now points back
	 * to the running partition — not the newly written one.  Use the boot
	 * partition instead when it differs from what is currently running.
	 */
	const esp_partition_t *boot    = esp_ota_get_boot_partition();
	const esp_partition_t *running = esp_ota_get_running_partition();
	if (boot != running) {
		return boot;
	}
	return esp_ota_get_next_update_partition(NULL);
}

/**
 * Virtual image layout served to mcumgr in place of a real MCUboot image:
 *
 *   [0 .. IMAGE_HEADER_SIZE)           struct image_header
 *   [IMAGE_HEADER_SIZE .. +4)           struct image_tlv_info  (IMAGE_TLV_INFO_MAGIC)
 *   [IMAGE_HEADER_SIZE+4 .. +4)         struct image_tlv       (IMAGE_TLV_SHA256, len=32)
 *   [IMAGE_HEADER_SIZE+8 .. +32)        uint8_t hash[32]       (app_elf_sha256)
 *
 * ih_img_size is 0, so mcumgr computes data_off = ih_hdr_size + 0 = IMAGE_HEADER_SIZE,
 * which lands exactly on the tlv_info — satisfying the TLV scan without requiring
 * a real MCUboot image in flash.
 */
struct virtual_img {
	struct image_header   hdr;
	struct image_tlv_info tlv_info;
	struct image_tlv      tlv_hash;
	uint8_t               hash[IMAGE_HASH_LEN];
};

static bool build_virtual_image(int slot, struct virtual_img *out)
{
	const esp_partition_t *part = slot_to_partition(slot);
	if (!part) {
		return false;
	}

	esp_app_desc_t desc;
	if (esp_ota_get_partition_description(part, &desc) != ESP_OK) {
		return false;
	}

	memset(out, 0, sizeof(*out));

	int major = 0;
	int minor = 0;
	int patch = 0;
	(void)sscanf(desc.version, "%d.%d.%d", &major, &minor, &patch);

	out->hdr.ih_magic         = IMAGE_MAGIC;
	out->hdr.ih_hdr_size      = IMAGE_HEADER_SIZE;
	out->hdr.ih_img_size      = 0;
	out->hdr.ih_ver.iv_major    = (uint8_t)major;
	out->hdr.ih_ver.iv_minor    = (uint8_t)minor;
	out->hdr.ih_ver.iv_revision = (uint16_t)patch;

	out->tlv_info.it_magic   = IMAGE_TLV_INFO_MAGIC;
	out->tlv_info.it_tlv_tot = (uint16_t)(sizeof(struct image_tlv_info)
			+ sizeof(struct image_tlv) + IMAGE_HASH_LEN);

	out->tlv_hash.it_type = IMAGE_TLV_SHA256;
	out->tlv_hash.it_len  = IMAGE_HASH_LEN;

	memcpy(out->hash, desc.app_elf_sha256, IMAGE_HASH_LEN);
	return true;
}

/* -------------------------------------------------------------------------
 * img_mgmt_impl_* — called directly by the mcumgr image management core.
 * ---------------------------------------------------------------------- */

int img_mgmt_impl_erase_slot(void)
{
	const esp_partition_t *part = slot_to_partition(1);
	if (!part) {
		return MGMT_ERR_EUNKNOWN;
	}
	esp_err_t err = esp_partition_erase_range(part, 0, part->size);
	return (err == ESP_OK) ? 0 : MGMT_ERR_EUNKNOWN;
}

int img_mgmt_impl_write_pending(int slot, bool permanent)
{
	(void)permanent;

	if (s_ota.handle) {
		esp_err_t err = esp_ota_end(s_ota.handle);
		s_ota.handle = 0;
		if (err != ESP_OK) {
			ESP_LOGE(TAG, "esp_ota_end failed: %s",
					esp_err_to_name(err));
			return MGMT_ERR_EUNKNOWN;
		}
	}

	const esp_partition_t *part = slot_to_partition(slot);
	if (!part) {
		return MGMT_ERR_EUNKNOWN;
	}

	esp_err_t err = esp_ota_set_boot_partition(part);
	return (err == ESP_OK) ? 0 : MGMT_ERR_EUNKNOWN;
}

int img_mgmt_impl_write_confirmed(void)
{
	esp_err_t err = esp_ota_mark_app_valid_cancel_rollback();
	return (err == ESP_OK) ? 0 : MGMT_ERR_EUNKNOWN;
}

int img_mgmt_impl_read(int slot, unsigned int offset, void *dst,
		unsigned int num_bytes)
{
	struct virtual_img vimg;
	if (build_virtual_image(slot, &vimg)) {
		if ((size_t)offset + num_bytes <= sizeof(vimg)) {
			memcpy(dst, (const uint8_t *)&vimg + offset, num_bytes);
			return 0;
		}
	}

	const esp_partition_t *part = slot_to_partition(slot);
	if (!part) {
		return MGMT_ERR_EUNKNOWN;
	}

	esp_err_t err = esp_partition_read(part, offset, dst, num_bytes);
	return (err == ESP_OK) ? 0 : MGMT_ERR_EUNKNOWN;
}

int img_mgmt_impl_write_image_data(unsigned int offset, const void *data,
		unsigned int num_bytes, bool last)
{
	if (!s_ota.handle) {
		const esp_partition_t *part =
				esp_ota_get_next_update_partition(NULL);
		if (!part) {
			return MGMT_ERR_EUNKNOWN;
		}
		esp_err_t err = esp_ota_begin(part, OTA_SIZE_UNKNOWN,
				&s_ota.handle);
		if (err != ESP_OK) {
			return MGMT_ERR_EUNKNOWN;
		}
	}

	esp_err_t err = esp_ota_write_with_offset(s_ota.handle, data,
			num_bytes, offset);
	if (err != ESP_OK) {
		return MGMT_ERR_EUNKNOWN;
	}

	if (last) {
		return img_mgmt_impl_write_pending(1, false);
	}
	return 0;
}

int img_mgmt_impl_swap_type(int slot)
{
	(void)slot;

	const esp_partition_t *part = esp_ota_get_next_update_partition(NULL);
	if (!part) {
		return IMG_MGMT_SWAP_TYPE_NONE;
	}

	esp_ota_img_states_t state;
	esp_err_t err = esp_ota_get_state_partition(part, &state);
	if (err != ESP_OK) {
		return IMG_MGMT_SWAP_TYPE_NONE;
	}

	switch (state) {
	case ESP_OTA_IMG_NEW:
	case ESP_OTA_IMG_PENDING_VERIFY:
		return IMG_MGMT_SWAP_TYPE_TEST;
	case ESP_OTA_IMG_VALID:
		return IMG_MGMT_SWAP_TYPE_NONE;
	case ESP_OTA_IMG_INVALID:
	case ESP_OTA_IMG_ABORTED:
	default:
		return IMG_MGMT_SWAP_TYPE_NONE;
	}
}

int img_mgmt_impl_upload_inspect(const struct img_mgmt_upload_req *req,
		struct img_mgmt_upload_action *action,
		const char **errstr)
{
	(void)errstr;

	action->size        = req->size;
	action->write_bytes = (int)req->data_len;
	action->proceed     = true;
	action->erase       = false;
	return 0;
}

int img_mgmt_impl_erased_val(int slot, uint8_t *erased_val)
{
	(void)slot;
	*erased_val = 0xFFU;
	return 0;
}

int img_mgmt_impl_erase_image_data(unsigned int off, unsigned int num_bytes)
{
	const esp_partition_t *part = esp_ota_get_next_update_partition(NULL);
	if (!part) {
		return MGMT_ERR_EUNKNOWN;
	}

	esp_err_t err = esp_partition_erase_range(part, off, num_bytes);
	return (err == ESP_OK) ? 0 : MGMT_ERR_EUNKNOWN;
}

int img_mgmt_impl_erase_if_needed(uint32_t off, uint32_t len)
{
	(void)off;
	(void)len;
	return 0;
}

int img_mgmt_impl_log_upload_start(int status)
{
	(void)status;
	return 0;
}

int img_mgmt_impl_log_upload_done(int status, const uint8_t *hashp)
{
	(void)status;
	(void)hashp;
	return 0;
}

int img_mgmt_impl_log_pending(int status, const uint8_t *hash)
{
	(void)status;
	(void)hash;
	return 0;
}

int img_mgmt_impl_log_confirm(int status, const uint8_t *hash)
{
	(void)status;
	(void)hash;
	return 0;
}
