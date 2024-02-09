/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/ble.h"

#include <errno.h>
#include <string.h>

#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/ble_hs_pvcy.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "host/util/util.h"

#include "libmcu/compiler.h"
#include "libmcu/assert.h"

LIBMCU_ASSERT(BLE_GAP_EVT_MAX < UINT8_MAX);

#if !defined(BLE_LOG_DEBUG)
#define BLE_LOG_DEBUG(...)
#endif
#if !defined(BLE_LOG_INFO)
#define BLE_LOG_INFO(...)
#endif
#if !defined(BLE_LOG_WARN)
#define BLE_LOG_WARN(...)
#endif
#if !defined(BLE_LOG_ERROR)
#define BLE_LOG_ERROR(...)
#endif

struct ble {
	struct ble_api api;

	ble_event_callback_t gap_event_callback;
	ble_event_callback_t gatt_event_callback;

	struct {
		enum ble_adv_mode mode;
		uint16_t min_ms;
		uint16_t max_ms;
		uint32_t duration_ms;

		struct ble_adv_payload payload;
		struct ble_adv_payload scan_response;
	} adv;

	uint8_t addr_type;
	uint8_t addr[BLE_ADDR_LEN];
	uint16_t connection_handle;
	volatile bool ready;
};

struct gatt_characteristic_handler {
	uint16_t handle;
	ble_gatt_characteristic_handler func;
	void *user_ctx;
};

struct ble_gatt_service {
	struct ble_gatt_svc_def base[2]; /* +1 entry wasted to represent the
	                                    end of services */
	struct {
		void *next;
		uint16_t len;
		uint16_t cap;
	} free;

	struct {
		uint8_t index;
		uint8_t nr_max;
		struct gatt_characteristic_handler *handlers;
	} characteristics;
};

static struct ble *onair;
static int adv_start(struct ble *self);
static int adv_stop(struct ble *self);

static void svc_mem_init(struct ble_gatt_service *svc, void *mem, uint16_t memsize)
{
	svc->free.cap = (uint16_t)(memsize - ALIGN(sizeof(*svc), sizeof(intptr_t)));
	svc->free.next = &((uint8_t *)mem)[ALIGN(sizeof(*svc), sizeof(intptr_t))];
}

static void *svc_mem_alloc(struct ble_gatt_service *svc, uint16_t size)
{
	uint16_t left = svc->free.cap - svc->free.len;
	void *p = NULL;

	size = ALIGN(size, sizeof(intptr_t));

	if (size <= left) {
		p = svc->free.next;
		svc->free.len += size;
		svc->free.next = &((uint8_t *)svc->free.next)[size];
	}

	return p;
}

static enum ble_device_addr read_device_address(uint8_t addr[BLE_ADDR_LEN])
{
	ble_addr_t t;

	int rc = ble_hs_id_infer_auto(0, &t.type);
	if (rc != 0) {
		BLE_LOG_ERROR("error determining address type; rc=%d", rc);
		assert(0);
	}

	rc = ble_hs_id_copy_addr(t.type, t.val, NULL);
	if (rc != 0) {
		BLE_LOG_ERROR("error reading address; rc=%d", rc);
		assert(0);
	}

	memcpy(addr, t.val, BLE_ADDR_LEN);

	if (BLE_ADDR_IS_STATIC(&t)) {
		return BLE_ADDR_STATIC_RPA;
	} else if (BLE_ADDR_IS_RPA(&t)) {
		return BLE_ADDR_PRIVATE_RPA;
	} else if (BLE_ADDR_IS_NRPA(&t)) {
		return BLE_ADDR_PRIVATE_NRPA;
	}

	return BLE_ADDR_PUBLIC;
}

static const struct gatt_characteristic_handler *get_chr_handler(
		const struct ble_gatt_service *svc,
		const struct ble_gatt_chr_def *chr)
{
	for (uint8_t i = 0; svc->base[0].characteristics != NULL &&
			svc->base[0].characteristics[i].uuid != NULL &&
			i < svc->characteristics.nr_max; i++) {
		if (&svc->base[0].characteristics[i] == chr) {
			return &svc->characteristics.handlers[i];
		}
	}

	return NULL;
}

static int on_characteristic_request(uint16_t conn_handle, uint16_t attr_handle,
			  struct ble_gatt_access_ctxt *ctxt, void *arg)
{
	struct ble_gatt_service *svc = (struct ble_gatt_service *)arg;

	const struct gatt_characteristic_handler *handler =
			get_chr_handler(svc, ctxt->chr);

	if (handler && handler->func) {
		struct ble_handler_context ctx = {
			.event_type = ctxt->op,
			.ctx = ctxt,
		};
		handler->func(&ctx, ctxt->om->om_data, ctxt->om->om_len,
				handler->user_ctx);
	}

	return 0;
}

static int on_gap_event(struct ble_gap_event *event, void *arg)
{
	struct ble *iface = (struct ble *)arg;
	struct ble_gap_conn_desc desc;
	enum ble_gap_evt evt = BLE_GAP_EVT_UNKNOWN;

	switch (event->type) {
	case BLE_GAP_EVENT_CONNECT:
		evt = BLE_GAP_EVT_CONNECTED;
		if (event->connect.status == 0) {
			iface->connection_handle = event->connect.conn_handle;
		}
		break;
	case BLE_GAP_EVENT_DISCONNECT:
		evt = BLE_GAP_EVT_DISCONNECTED;
		iface->connection_handle = 0;
		break;
	case BLE_GAP_EVENT_ADV_COMPLETE:
		evt = BLE_GAP_EVT_ADV_COMPLETE;
		if (event->adv_complete.reason != 0 &&
				event->adv_complete.reason != BLE_HS_ETIMEOUT) {
			evt = BLE_GAP_EVT_ADV_SUSPENDED;
			BLE_LOG_ERROR("adv stopped: %d",
					event->adv_complete.reason);
			if (iface->adv.duration_ms == BLE_HS_FOREVER) {
				adv_start(iface);
			}
		}
		break;
	case BLE_GAP_EVENT_CONN_UPDATE:
	case BLE_GAP_EVENT_MTU:
	default:
		evt = event->type;
		break;
	}

	if (iface && iface->gap_event_callback) {
		iface->gap_event_callback(iface, evt, 0);
	}

	return 0;
}

static void on_gatt_register(struct ble_gatt_register_ctxt *ctxt, void *arg)
{
	char buf[BLE_UUID_STR_LEN];

	switch (ctxt->op) {
	case BLE_GATT_REGISTER_OP_SVC:
		BLE_LOG_DEBUG("registered service %s with handle=%d\n",
				ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
				ctxt->svc.handle);
		break;
	case BLE_GATT_REGISTER_OP_CHR:
		BLE_LOG_DEBUG("registered characteristic %s with "
				"def_handle=%d val_handle=%d\n",
				ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
				ctxt->chr.def_handle, ctxt->chr.val_handle);
		break;
	case BLE_GATT_REGISTER_OP_DSC:
		BLE_LOG_DEBUG("registered descriptor %s with handle=%d\n",
				ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
				ctxt->dsc.handle);
		break;
	default:
		assert(0);
		break;
	}
}

static void on_reset(int reason)
{
	BLE_LOG_INFO("reset reason: %d", reason);
}

static void on_sync(void)
{
	assert(onair);

	int rc = ble_hs_util_ensure_addr(0);
	assert(rc == 0);

	if (onair->addr_type == BLE_ADDR_PRIVATE_RPA ||
			onair->addr_type == BLE_ADDR_PRIVATE_NRPA) {
		uint8_t type = onair->addr_type - 1;/*1:RPA, 2:NRPA, 0:disable*/
#if 0
		extern int ble_hs_pvcy_rpa_config(uint8_t enable);
		rc = ble_hs_pvcy_rpa_config(type);
		assert(rc == 0);
#endif
	}

	onair->ready = true;
}

static void ble_spp_server_host_task(void *param)
{
	nimble_port_run();
	nimble_port_freertos_deinit();
}

static int adv_set_interval(struct ble *self, uint16_t min_ms, uint16_t max_ms)
{
	assert(min_ms >= BLE_ADV_MIN_INTERVAL_MS &&
			max_ms <= BLE_ADV_MAX_INTERVAL_MS);

	self->adv.min_ms = min_ms;
	self->adv.max_ms = max_ms;

	return 0;
}

static int adv_set_duration(struct ble *self, uint32_t msec)
{
	self->adv.duration_ms = msec;
	return 0;
}

static void register_gap_event_callback(struct ble *self,
					ble_event_callback_t cb)
{
	self->gap_event_callback = cb;
}

static void register_gatt_event_callback(struct ble *self,
					 ble_event_callback_t cb)
{
	self->gatt_event_callback = cb;
}

static int adv_set_payload(struct ble *self,
			   const struct ble_adv_payload *payload)
{
	memcpy(&self->adv.payload, payload, sizeof(*payload));
	return 0;
}

static int adv_set_scan_response(struct ble *self,
				 const struct ble_adv_payload *payload)
{
	memcpy(&self->adv.scan_response, payload, sizeof(*payload));
	return 0;
}

static int adv_start(struct ble *self)
{
	if (!self->ready) {
		/* This should be called once all GATT services registered. */
		nimble_port_freertos_init(ble_spp_server_host_task);

		while (!self->ready) { /* FIXME: remove blocking loop */
			/* waiting to be ready */
		}
	}

	int rc = ble_gap_adv_set_data(self->adv.payload.payload,
			       self->adv.payload.index);
	rc |= ble_gap_adv_rsp_set_data(self->adv.scan_response.payload,
			       self->adv.scan_response.index);
	if (rc != 0) {
		return -EINVAL;
	}

	struct ble_gap_adv_params adv_params = {
		.conn_mode = BLE_GAP_CONN_MODE_UND,
		.disc_mode = BLE_GAP_DISC_MODE_GEN,
		.itvl_min = self->adv.min_ms * 1000 / BLE_HCI_ADV_ITVL,
		.itvl_max = self->adv.max_ms * 1000 / BLE_HCI_ADV_ITVL,
	};

	switch (self->adv.mode) {
	case BLE_ADV_DIRECT_IND:
		adv_params.conn_mode = BLE_GAP_CONN_MODE_DIR;
		adv_params.disc_mode = BLE_GAP_DISC_MODE_LTD;
		break;
	case BLE_ADV_NONCONN_IND:
		adv_params.conn_mode = BLE_GAP_CONN_MODE_NON;
		adv_params.disc_mode = BLE_GAP_DISC_MODE_NON;
		break;
	case BLE_ADV_SCAN_IND:
		adv_params.conn_mode = BLE_GAP_CONN_MODE_NON;
		break;
	default:
	case BLE_ADV_IND:
		break;
	}

	rc = ble_gap_adv_start(self->addr_type, NULL,
			(int32_t)self->adv.duration_ms,
			&adv_params, on_gap_event, self);
	if (rc != 0) {
		BLE_LOG_ERROR("adv failure: %d", rc);
		return -EFAULT;
	}

	enum ble_device_addr type = read_device_address(self->addr);
	if (type != self->addr_type) {
		BLE_LOG_WARN("addr type mismatch: %d expected but %d",
				self->addr_type, type);
	}

	return 0;
}

static int adv_stop(struct ble *self)
{
	if (!ble_gap_adv_active()) {
		return 0;
	}

	return ble_gap_adv_stop();
}

static int adv_init(struct ble *self, enum ble_adv_mode mode)
{
	memset(&self->adv, 0, sizeof(self->adv));

	self->adv.mode = mode;

	self->adv.min_ms = 0;
	self->adv.max_ms = 0;
	self->adv.duration_ms = BLE_HS_FOREVER;

	return 0;
}

static void copy_uuid(ble_uuid_any_t *p, const uint8_t *uuid, uint8_t uuid_len)
{
	uint8_t *t = p->u128.value;
	p->u.type = BLE_UUID_TYPE_128;

	if (uuid_len == 2) {
		p->u.type = BLE_UUID_TYPE_16;
		t = (uint8_t *)&p->u16.value;
	} else if (uuid_len == 4) {
		p->u.type = BLE_UUID_TYPE_32;
		t = (uint8_t *)&p->u32.value;
	}

	memcpy(t, uuid, uuid_len);
}

static struct ble_gatt_service *gatt_create_service(void *mem, uint16_t memsize,
		const uint8_t *uuid, uint8_t uuid_len,
		bool primary, uint8_t nr_chrs)
{
	assert(mem != NULL);
	assert(memsize > (uint16_t)(sizeof(struct ble_gatt_service) +
			(nr_chrs+1/*null desc*/) * sizeof(struct ble_gatt_chr_def) +
			nr_chrs * sizeof(struct ble_gatt_dsc_def) * 6/*max.desc*/ +
			nr_chrs * sizeof(struct gatt_characteristic_handler)));
	assert(uuid != NULL);
	assert(uuid_len == 2 || uuid_len == 4 || uuid_len == 16);

	memset(mem, 0, memsize);
	struct ble_gatt_service *svc = (struct ble_gatt_service *)mem;
	svc_mem_init(svc, mem, memsize);

	ble_uuid_any_t *uuid_converted = (ble_uuid_any_t *)svc_mem_alloc(svc, uuid_len + 1);
	copy_uuid(uuid_converted, uuid, uuid_len);

	struct ble_gatt_chr_def *chrs = (struct ble_gatt_chr_def *)
			svc_mem_alloc(svc, (nr_chrs+1) * sizeof(*chrs));

	struct gatt_characteristic_handler *handlers =
			(struct gatt_characteristic_handler *)
			svc_mem_alloc(svc, nr_chrs * sizeof(*handlers));

	svc->base[0].type = primary?
		BLE_GATT_SVC_TYPE_PRIMARY : BLE_GATT_SVC_TYPE_SECONDARY;
	svc->base[0].uuid = (const ble_uuid_t *)uuid_converted;
	svc->base[0].characteristics = chrs;
	svc->characteristics.nr_max = nr_chrs;
	svc->characteristics.handlers = handlers;

	return svc;
}

static const uint16_t *gatt_add_characteristic(struct ble_gatt_service *svc,
		const uint8_t *uuid, uint8_t uuid_len,
		struct ble_gatt_characteristic *chr)
{
	assert(svc->characteristics.index < svc->characteristics.nr_max);

	uint8_t i = svc->characteristics.index++;
	struct ble_gatt_chr_def *p = (struct ble_gatt_chr_def *)
			&svc->base[0].characteristics[i];

	ble_uuid_any_t *uuid_converted = (ble_uuid_any_t *)svc_mem_alloc(svc, uuid_len + 1);
	copy_uuid(uuid_converted, uuid, uuid_len);

	svc->characteristics.handlers[i].func = chr->handler;
	svc->characteristics.handlers[i].user_ctx = chr->user_ctx;

	p->uuid = (const ble_uuid_t *)uuid_converted;
	p->access_cb = on_characteristic_request;
	p->arg = svc;
	p->val_handle = &svc->characteristics.handlers[i].handle;

	if (chr->op & BLE_GATT_OP_READ) {
		p->flags |= BLE_GATT_CHR_F_READ;
	}
	if (chr->op & BLE_GATT_OP_WRITE) {
		p->flags |= BLE_GATT_CHR_F_WRITE;
	}
	if (chr->op & BLE_GATT_OP_NOTIFY) {
		p->flags |= BLE_GATT_CHR_F_NOTIFY;
	}
	if (chr->op & BLE_GATT_OP_INDICATE) {
		p->flags |= BLE_GATT_CHR_F_INDICATE;
	}

	return p->val_handle;
}

static int gatt_register_service(struct ble *ble, struct ble_gatt_service *svc)
{
	(void)ble;

	int rc = ble_gatts_count_cfg(svc->base) |
		ble_gatts_add_svcs(svc->base);

	return rc;
}

static int gatt_response(struct ble_handler_context *ctx,
		const void *data, uint16_t datasize)
{
	struct ble_gatt_access_ctxt *p = (struct ble_gatt_access_ctxt *)ctx->ctx;
	return os_mbuf_append(p->om, data, sizeof(datasize));
}

static int gatt_notify(struct ble *self, const void *attr_handle,
		const void *data, uint16_t datasize)
{
	uint16_t attr = *((uint16_t *)attr_handle);
	struct os_mbuf *om;
	om = ble_hs_mbuf_from_flat(data, datasize);

	return ble_gattc_notify_custom(self->connection_handle, attr, om);
}

static enum ble_device_addr get_device_address(struct ble *iface,
		uint8_t addr[BLE_ADDR_LEN])
{
	memcpy(addr, iface->addr, BLE_ADDR_LEN);
	return (enum ble_device_addr)iface->addr_type;
}

static void initialize(struct ble *iface)
{
	/* ESP_ERROR_CHECK(esp_nimble_hci_and_controller_init()); */
	nimble_port_init();

	ble_hs_cfg.reset_cb = on_reset;
	ble_hs_cfg.sync_cb = on_sync;
	ble_hs_cfg.gatts_register_cb = on_gatt_register;
	ble_hs_cfg.gatts_register_arg = iface;
	ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

	if (iface->addr_type == BLE_ADDR_PRIVATE_RPA ||
			iface->addr_type == BLE_ADDR_PRIVATE_NRPA) {
		ble_hs_cfg.sm_our_key_dist = BLE_SM_PAIR_KEY_DIST_ID |
				BLE_SM_PAIR_KEY_DIST_ENC;
		ble_hs_cfg.sm_their_key_dist = BLE_SM_PAIR_KEY_DIST_ID |
				BLE_SM_PAIR_KEY_DIST_ENC;
	}

	ble_svc_gap_init();
	ble_svc_gatt_init();
	ble_svc_gap_device_name_set(BLE_DEFAULT_DEVICE_NAME);
}

static int enable_device(struct ble *self,
		enum ble_device_addr addr_type, uint8_t addr[BLE_ADDR_LEN])
{
	if (!onair) {
		self->addr_type = addr_type;
		if (addr) {
			memcpy(self->addr, addr, sizeof(self->addr));
		}

		initialize(self);
		onair = self;
	}

	return 0;
}

static int disable_device(struct ble *self)
{
	int rc = 0;

	if (self->addr_type == BLE_ADDR_PRIVATE_RPA ||
			self->addr_type == BLE_ADDR_PRIVATE_NRPA) {
#if 0
		extern int ble_hs_pvcy_rpa_config(uint8_t enable);
		rc = ble_hs_pvcy_rpa_config(0);
#endif
	}

	rc |= nimble_port_stop();
	nimble_port_deinit();
	/* rc |= esp_nimble_hci_and_controller_deinit(); */
	onair = NULL;

	return rc;
}

struct ble *ble_create(int id)
{
	if (id != 0) {
		return NULL;
	}

	static struct ble self = {
		.api = {
			.enable = enable_device,
			.disable = disable_device,
			.register_gap_event_callback =
				register_gap_event_callback,
			.register_gatt_event_callback =
				register_gatt_event_callback,
			.get_device_address = get_device_address,

			.adv_init = adv_init,
			.adv_set_interval = adv_set_interval,
			.adv_set_duration = adv_set_duration,
			.adv_set_payload = adv_set_payload,
			.adv_set_scan_response = adv_set_scan_response,
			.adv_start = adv_start,
			.adv_stop = adv_stop,

			.gatt_create_service = gatt_create_service,
			.gatt_add_characteristic = gatt_add_characteristic,
			.gatt_register_service = gatt_register_service,
			.gatt_response = gatt_response,
			.gatt_notify = gatt_notify,
		},
	};

	return &self;
}

void ble_destroy(struct ble *iface)
{
	memset(iface, 0, sizeof(*iface));
}
