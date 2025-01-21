/*
 * SPDX-FileCopyrightText: 2022 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#pragma push_macro("BLE_GAP_EVT_CONNECTED")
#pragma push_macro("BLE_GAP_EVT_DISCONNECTED")
#define BLE_GAP_EVT_CONNECTED		LIBMCU_BLE_GAP_EVT_CONNECTED
#define BLE_GAP_EVT_DISCONNECTED	LIBMCU_BLE_GAP_EVT_DISCONNECTED
#include "libmcu/ble.h"
#pragma pop_macro("BLE_GAP_EVT_DISCONNECTED")
#pragma pop_macro("BLE_GAP_EVT_CONNECTED")

#include <errno.h>
#include <string.h>

#include "ble.h"
#include "ble_gap.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "ble_advertising.h"
#include "nrf_ble_gatt.h"
#include "ble_srv_common.h"

#include "libmcu/compiler.h"
#include "libmcu/assert.h"

#if !defined(BLE_LOG_INFO)
#define BLE_LOG_INFO(...)
#endif
#if !defined(BLE_LOG_WARN)
#define BLE_LOG_WARN(...)
#endif
#if !defined(BLE_LOG_ERROR)
#define BLE_LOG_ERROR(...)
#endif

static_assert(BLE_GAP_EVT_MAX < UINT8_MAX,
		"BLE_GAP_EVT_MAX must be less than UINT8_MAX");

#define BLE_PRIORITY			3
#define ADV_INTERVAL_UNIT_THOUSANDTH	625
#define DEFAULT_ADV_INTERVAL_MS		180
#define DEFAULT_MTU_SIZE		23
#define UUID_128BIT_SIZE_BYTES		16
#define MAX_ATTR_VALUE_LEN		(NRF_SDH_BLE_GATT_MAX_MTU_SIZE - 3)

enum ble_tag {
	CONN_CFG_TAG			= 0,
	CONN_TAG			= 1,
};

struct gatt_characteristic_handler {
	ble_gatts_char_handles_t handle;
	ble_gatt_characteristic_handler func;
	void *user_ctx;
};

struct ble_gatt_service {
	struct ble_gatt_service *next;

	uint16_t handle;
	ble_uuid_t uuid;

	struct {
		uint8_t index;
		uint8_t capacity;
		struct gatt_characteristic_handler handlers[1];
	} characteristics;
};

struct ble {
	struct ble_api api;

	ble_event_callback_t gap_event_callback;
	ble_event_callback_t gatt_event_callback;

	struct {
		ble_advertising_t handle;

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

	struct ble_gatt_service *svc;

	bool onair;
};

static struct ble static_instance;
static int adv_start(struct ble *self);
static int adv_stop(struct ble *self);

static void on_adv_event(ble_adv_evt_t ble_adv_evt)
{
	switch (ble_adv_evt) {
	case BLE_ADV_EVT_FAST:
		BLE_LOG_INFO("Advertising started");
		break;
	case BLE_ADV_EVT_IDLE:
		BLE_LOG_INFO("Idle. sleep to save power");
		break;
        default:
		BLE_LOG_INFO("ADV event: %d", ble_adv_evt);
		break;
	}
}

static void on_gatt_event(nrf_ble_gatt_t *pgatt, nrf_ble_gatt_evt_t const *pevt)
{
	switch (pevt->evt_id) {
	case NRF_BLE_GATT_EVT_ATT_MTU_UPDATED:
		BLE_LOG_INFO("MTU on connection %x changed to %d",
			pevt->conn_handle, pevt->params.att_mtu_effective);
		break;
	}
	BLE_LOG_INFO("GATT EVT: %d", pevt->evt_id);
}

static struct gatt_characteristic_handler *find_chr_by_handle(struct ble *self,
		uint16_t handle)
{
	struct ble_gatt_service *svc = self->svc;

	while (svc) {
		for (uint8_t i = 0; i < svc->characteristics.capacity; i++) {
			if (svc->characteristics.handlers[i].handle.value_handle
					== handle) {
				return &svc->characteristics.handlers[i];
			}
		}
		svc = svc->next;
	}

	return NULL;
}

static int response_to_request(struct ble *self, uint8_t type,
		const void *data, uint16_t datasize)
{
	ble_gatts_rw_authorize_reply_params_t reply_params = {
		.type = BLE_GATTS_AUTHORIZE_TYPE_READ,
		.params.read = { /* union struct of read and write */
			.gatt_status = BLE_GATT_STATUS_SUCCESS,
			.len = datasize,
			.p_data = (const uint8_t *)data,
			.update = true,
		},
	};

	if (type == BLE_GATT_EVT_WRITE_CHR) {
		reply_params.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
	}

	if (sd_ble_gatts_rw_authorize_reply(self->connection_handle,
				&reply_params) != NRF_SUCCESS) {
		BLE_LOG_ERROR("response failure");
		return -EINVAL;
	}

	return 0;
}

static void handle_rw_request(struct ble *self, const ble_evt_t *evt)
{
	struct gatt_characteristic_handler *p = find_chr_by_handle(self,
			evt->evt.gatts_evt.params.authorize_request.request.read.handle);
	const void *data = NULL;
	uint16_t datasize = 0;

	if (p == NULL) {
		BLE_LOG_ERROR("handle not found");
		return;
	}

	struct ble_handler_context ctx = {
		.event_type = BLE_GATT_EVT_READ_CHR,
		.ctx = self,
	};

	if (evt->evt.gatts_evt.params.authorize_request.type ==
			BLE_GATTS_AUTHORIZE_TYPE_WRITE) {
		ctx.event_type = BLE_GATT_EVT_WRITE_CHR;
		data = evt->evt.gatts_evt.params.authorize_request.request.write.data;
		datasize = evt->evt.gatts_evt.params.authorize_request.request.write.len;
	}

	if (p->func) {
		(*p->func)(&ctx, data, datasize, p->user_ctx);
	}

	if (ctx.event_type == BLE_GATT_EVT_WRITE_CHR) {
		response_to_request(self, ctx.event_type, data, datasize);
	}
}

static enum ble_gap_evt process_gatts_event(struct ble *self, const ble_evt_t *evt)
{
	switch (evt->header.evt_id) {
	case BLE_GATTS_EVT_WRITE:
		return BLE_GATT_EVT_WRITE_CHR;
	case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
		handle_rw_request(self, evt);
		return BLE_GATT_EVT_READ_CHR;
	default:
	case BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST:
	case BLE_GATTS_EVT_TIMEOUT:
	case BLE_GATTC_EVT_TIMEOUT:
		return evt->header.evt_id;
	}
}

static enum ble_gap_evt process_gattc_event(struct ble *self, const ble_evt_t *evt)
{
	switch (evt->header.evt_id) {
	case BLE_GATTC_EVT_TIMEOUT:
	default:
		return evt->header.evt_id;
	}
}

static enum ble_gap_evt process_gap_event(struct ble *self, const ble_evt_t *evt)
{
	switch (evt->header.evt_id) {
	case BLE_GAP_EVT_ADV_SET_TERMINATED:
		return BLE_GAP_EVT_ADV_COMPLETE;
	case BLE_GAP_EVT_CONNECTED:
		self->connection_handle = evt->evt.gap_evt.conn_handle;
		return LIBMCU_BLE_GAP_EVT_CONNECTED;
	case BLE_GAP_EVT_DISCONNECTED:
		return LIBMCU_BLE_GAP_EVT_DISCONNECTED;
	case BLE_GAP_EVT_DATA_LENGTH_UPDATE:
		return BLE_GAP_EVT_MTU;
	default:
	case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
	case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
	case BLE_GAP_EVT_AUTH_KEY_REQUEST:
	case BLE_GAP_EVT_LESC_DHKEY_REQUEST:
	case BLE_GAP_EVT_AUTH_STATUS:
		BLE_LOG_INFO("EVT: %d", evt->header.evt_id);
		return evt->header.evt_id;
	}
}

static void on_ble_events(ble_evt_t const *p_ble_evt, void *p_context)
{
	struct ble *self = (struct ble *)p_context;
	enum ble_gap_evt evt = BLE_GAP_EVT_UNKNOWN;
	bool gatt = true;

	if (p_ble_evt->header.evt_id >= BLE_GATTC_EVT_BASE &&
			p_ble_evt->header.evt_id <= BLE_GATTC_EVT_LAST) {
		evt = process_gattc_event(self, p_ble_evt);
	} else if (p_ble_evt->header.evt_id >= BLE_GATTS_EVT_BASE &&
			p_ble_evt->header.evt_id <= BLE_GATTS_EVT_LAST) {
		evt = process_gatts_event(self, p_ble_evt);
	} else {
		evt = process_gap_event(self, p_ble_evt);
		gatt = false;
	}

	if (gatt) {
		if (self && self->gatt_event_callback) {
			self->gatt_event_callback(self, evt, 0);
		}
	} else if (self && self->gap_event_callback) {
		self->gap_event_callback(self, evt, 0);
	}
}

static void on_error(uint32_t error_code)
{
	BLE_LOG_ERROR("ERR: %x", error_code);
}

static enum ble_device_addr read_device_address(uint8_t addr[BLE_ADDR_LEN])
{
	ble_gap_addr_t mac;

	if (sd_ble_gap_addr_get(&mac) != NRF_SUCCESS) {
		BLE_LOG_ERROR("error reading address");
		assert(0);
	}

	memcpy(addr, mac.addr, BLE_ADDR_LEN);

	switch (mac.addr_type) {
	case BLE_GAP_ADDR_TYPE_RANDOM_STATIC:
		return BLE_ADDR_STATIC_RPA;
	case BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE:
		return BLE_ADDR_PRIVATE_RPA;
	case BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_NON_RESOLVABLE:
		return BLE_ADDR_PRIVATE_NRPA;
	default:
	case BLE_GAP_ADDR_TYPE_PUBLIC:
		return BLE_ADDR_PUBLIC;
	}
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

static int initialize_advertising(struct ble *self,
		const ble_adv_modes_config_t *cfg)
{
	ble_advertising_t *handle = &self->adv.handle;

	memset(handle, 0, sizeof(*handle));

	handle->adv_mode_current = BLE_ADV_MODE_IDLE;
	handle->adv_modes_config = *cfg;
	handle->conn_cfg_tag = CONN_CFG_TAG;
	handle->evt_handler = on_adv_event;
	handle->error_handler = on_error;
	handle->current_slave_link_conn_handle = BLE_CONN_HANDLE_INVALID;
	handle->p_adv_data = &handle->adv_data;

	if (!handle->initialized) {
		handle->adv_handle = BLE_GAP_ADV_SET_HANDLE_NOT_SET;
	}

	handle->adv_params.primary_phy = BLE_GAP_PHY_1MBPS;
	handle->adv_params.properties.type =
		BLE_GAP_ADV_TYPE_CONNECTABLE_SCANNABLE_UNDIRECTED;
	handle->adv_params.p_peer_addr = NULL;
	handle->adv_params.filter_policy = BLE_GAP_ADV_FP_ANY;

	switch (self->adv.mode) {
	case BLE_ADV_DIRECT_IND:
		handle->adv_params.properties.type =
			BLE_GAP_ADV_TYPE_CONNECTABLE_NONSCANNABLE_DIRECTED;
		break;
	case BLE_ADV_NONCONN_IND:
		handle->adv_params.properties.type =
			BLE_GAP_ADV_TYPE_NONCONNECTABLE_NONSCANNABLE_UNDIRECTED;
		break;
	case BLE_ADV_SCAN_IND:
		handle->adv_params.properties.type =
			BLE_GAP_ADV_TYPE_NONCONNECTABLE_SCANNABLE_UNDIRECTED;
		break;
	default:
	case BLE_ADV_IND:
		break;
	}

	handle->initialized = true;

	return 0;
}

static int adv_start(struct ble *self)
{
	if (!nrf_sdh_is_enabled()) {
		return -ENODEV;
	}

	/* advertising data */
	self->adv.handle.adv_data.adv_data.p_data =
			self->adv.handle.enc_advdata[0];
	memcpy(self->adv.handle.adv_data.adv_data.p_data,
			self->adv.payload.payload, self->adv.payload.index);
	self->adv.handle.adv_data.adv_data.len = self->adv.payload.index;

	/* scan response data */
	self->adv.handle.adv_data.scan_rsp_data.p_data =
			self->adv.handle.enc_scan_rsp_data[0];
	memcpy(self->adv.handle.adv_data.scan_rsp_data.p_data,
			self->adv.scan_response.payload,
			self->adv.scan_response.index);
	self->adv.handle.adv_data.scan_rsp_data.len =
			self->adv.scan_response.index;

	self->adv.handle.adv_modes_config.ble_adv_fast_timeout =
		self->adv.duration_ms / 10;
	self->adv.handle.adv_modes_config.ble_adv_fast_interval =
		self->adv.min_ms * 1000UL / ADV_INTERVAL_UNIT_THOUSANDTH;

	uint32_t err = ble_advertising_start(&self->adv.handle,
			BLE_ADV_MODE_FAST);
	if (err != NRF_SUCCESS) {
		BLE_LOG_ERROR("advertising start failure: %x", err);
		return -ENETDOWN;
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
	sd_ble_gap_adv_stop(self->adv.handle.adv_handle);
	return 0;
}

static int adv_init(struct ble *self, enum ble_adv_mode mode)
{
	memset(&self->adv, 0, sizeof(self->adv));

	self->adv.mode = mode;

	self->adv.min_ms = DEFAULT_ADV_INTERVAL_MS;
	self->adv.max_ms = DEFAULT_ADV_INTERVAL_MS;
	self->adv.duration_ms = 0; /* forever */

	ble_adv_modes_config_t cfg = {
		.ble_adv_fast_enabled = true,
		.ble_adv_on_disconnect_disabled = true,
	};

	if (initialize_advertising(self, &cfg)) {
		BLE_LOG_ERROR("advertising init failure");
		return -EINVAL;
	}

	ble_advertising_conn_cfg_tag_set(&self->adv.handle, CONN_TAG);

	return 0;
}

static struct ble_gatt_service *gatt_create_service(void *mem, uint16_t memsize,
		const uint8_t *uuid, uint8_t uuid_len,
		bool primary, uint8_t nr_chrs)
{
	assert(mem != NULL);

	memset(mem, 0, memsize);
	struct ble_gatt_service *svc = (struct ble_gatt_service *)mem;
	svc->characteristics.capacity = nr_chrs;

	uint8_t svc_type = primary? BLE_GATTS_SRVC_TYPE_PRIMARY :
			BLE_GATTS_SRVC_TYPE_SECONDARY;

	/* 1. register a service UUID */
	svc->uuid = (ble_uuid_t) {
		.type = BLE_UUID_TYPE_BLE,
		.uuid = *((const uint16_t *)uuid),
	};

	if (uuid_len == UUID_128BIT_SIZE_BYTES) {
		if (sd_ble_uuid_vs_add((const ble_uuid128_t *)uuid,
				&svc->uuid.type) != NRF_SUCCESS) {
			BLE_LOG_ERROR("can not register uuid");
			return NULL;
		}
		svc->uuid.uuid = *((const uint16_t *)&uuid[12]);
	}

	/* 2. add a service */
	if (sd_ble_gatts_service_add(svc_type, &svc->uuid, &svc->handle)
			!= NRF_SUCCESS) {
		BLE_LOG_ERROR("service registration failure");
		return NULL;
	}

	return svc;
}

static int gatt_register_service(struct ble *ble, struct ble_gatt_service *svc)
{
	svc->next = ble->svc;
	ble->svc = svc;
}

static struct gatt_characteristic_handler *svc_chr_alloc(
		struct ble_gatt_service *svc)
{
	if (svc->characteristics.index >= svc->characteristics.capacity) {
		return NULL;
	}

	struct gatt_characteristic_handler *p =
		&svc->characteristics.handlers[svc->characteristics.index];

	svc->characteristics.index += 1;
	return p;
}

static const uint16_t *gatt_add_characteristic(struct ble_gatt_service *svc,
		const uint8_t *uuid, uint8_t uuid_len,
		struct ble_gatt_characteristic *chr)
{
	struct gatt_characteristic_handler *p = svc_chr_alloc(svc);

	if (p == NULL) {
		BLE_LOG_ERROR("Out of memory");
		return NULL;
	}

	p->func = chr->handler;
	p->user_ctx = chr->user_ctx;

	ble_add_char_params_t params = {
		.uuid = *((const uint16_t *)uuid),
		.uuid_type = BLE_UUID_TYPE_BLE,
		.max_len = MAX_ATTR_VALUE_LEN,
		.init_len = 1,
		.is_var_len = true,
		.read_access = SEC_OPEN,
		.write_access = SEC_OPEN,
		.cccd_write_access = SEC_OPEN,
		.is_defered_read = true,
		.is_defered_write = true,
	};

	if (uuid_len == UUID_128BIT_SIZE_BYTES) {
		params.uuid_type = svc->uuid.type;
		params.uuid = *((const uint16_t *)&uuid[12]);
#if 0 /* TODO: support 128-bit UUID characteristics */
		if (sd_ble_uuid_vs_add((const ble_uuid128_t *)uuid,
				&params.uuid_type) != NRF_SUCCESS) {
			BLE_LOG_ERROR("can not register uuid");
			return NULL;
		}
#endif
	}

	if (chr->op & BLE_GATT_OP_READ) {
		params.char_props.read = true;
	}
	if (chr->op & BLE_GATT_OP_WRITE) {
		params.char_props.write = true;
		params.char_props.write_wo_resp = true;
	}
	if (chr->op & BLE_GATT_OP_NOTIFY) {
		params.char_props.notify = true;
	}
	if (chr->op & BLE_GATT_OP_INDICATE) {
		params.char_props.indicate = true;
	}

	if (characteristic_add(svc->handle, &params, &p->handle)
			!= NRF_SUCCESS) {
		BLE_LOG_ERROR("characteristic registration failure");
		return NULL;
	}

	return &p->handle.value_handle;
}

static int gatt_response(struct ble_handler_context *ctx,
		const void *data, uint16_t datasize)
{
	struct ble *self = (struct ble *)ctx->ctx;
	return response_to_request(self, ctx->event_type, data, datasize);
}

static int gatt_notify(struct ble *self, const void *attr_handle,
		const void *data, uint16_t datasize)
{
	ble_gatts_hvx_params_t params = {
		.handle = *(const uint16_t *)attr_handle,
		.type = BLE_GATT_HVX_NOTIFICATION,
		.p_len = &datasize,
		.p_data = data,
	};

	if (sd_ble_gatts_hvx(self->connection_handle, &params)
			!= NRF_SUCCESS) {
		return -ENETUNREACH;
	}

	return 0;
}

static enum ble_device_addr get_device_address(struct ble *self,
		uint8_t addr[BLE_ADDR_LEN])
{
	memcpy(addr, self->addr, BLE_ADDR_LEN);
	return (enum ble_device_addr)self->addr_type;
}

static int initialize_softdevice(void)
{
	uint32_t ram_start = 0;

#if 0 /* This should be done first outside the library. */
	if (nrf_sdh_enable_request() != NRF_SUCCESS) {
		BLE_LOG_ERROR("softdevice not ready");
		return -ENOLINK;
	}
#endif
	if (nrf_sdh_ble_default_cfg_set(CONN_TAG, &ram_start) != NRF_SUCCESS) {
		BLE_LOG_ERROR("invalid softdevice configuration");
		return -EINVAL;
	}
	if (nrf_sdh_ble_enable(&ram_start) != NRF_SUCCESS) {
		BLE_LOG_ERROR("BLE start failure. Check RAM start address: %x",
				ram_start);
		return -EADDRNOTAVAIL;
	}

	NRF_SDH_BLE_OBSERVER(event_handle, BLE_PRIORITY,
			on_ble_events, &static_instance);

	return 0;
}

static int initialize_gap(struct ble *self)
{
	ble_gap_conn_sec_mode_t sec_mode;
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
	if (sd_ble_gap_device_name_set(&sec_mode,
			(const uint8_t *)BLE_DEFAULT_DEVICE_NAME,
			sizeof(BLE_DEFAULT_DEVICE_NAME)-1) != NRF_SUCCESS) {
		BLE_LOG_ERROR("can not set device name");
		return -EAGAIN;
	}

	ble_gap_privacy_params_t privacy_params = {
		.privacy_mode = BLE_GAP_PRIVACY_MODE_DEVICE_PRIVACY,
		.private_addr_type = BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE,
		.private_addr_cycle_s =
			BLE_GAP_DEFAULT_PRIVATE_ADDR_CYCLE_INTERVAL_S,
		.p_device_irk = NULL,
	};

	switch (self->addr_type) {
	case BLE_ADDR_PUBLIC:
		privacy_params.private_addr_type = BLE_GAP_ADDR_TYPE_PUBLIC;
		break;
	case BLE_ADDR_STATIC_RPA:
		privacy_params.private_addr_type =
			BLE_GAP_ADDR_TYPE_RANDOM_STATIC;
		break;
	case BLE_ADDR_PRIVATE_NRPA:
		privacy_params.private_addr_type =
			BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_NON_RESOLVABLE;
		break;
	default: /* fall through */
	case BLE_ADDR_PRIVATE_RPA:
		break;
	}

	if (sd_ble_gap_privacy_set(&privacy_params) != NRF_SUCCESS) {
		return -EAGAIN;
	}

	return 0;
}

static int initialize_gatt(struct ble *self)
{
	int rc = 0;

	NRF_BLE_GATT_DEF(gatt_handle);

	if (nrf_ble_gatt_init(&gatt_handle, on_gatt_event) != NRF_SUCCESS) {
		BLE_LOG_ERROR("GATT initialization failure");
		return -EIO;
	}
	if (nrf_ble_gatt_att_mtu_periph_set(&gatt_handle, DEFAULT_MTU_SIZE)
			!= NRF_SUCCESS) {
		BLE_LOG_ERROR("MTU configuration failure");
		return -EAGAIN;
	}

	return rc;
}

static int initialize(struct ble *self)
{
	int rc = 0;

	if ((rc = initialize_softdevice()) != 0 ||
			(rc = initialize_gap(self)) != 0 ||
			(rc = initialize_gatt(self)) != 0) {
		BLE_LOG_ERROR("Initialization failure %d", rc);
		/* return error */
	}

	return rc;
}

static int enable_device(struct ble *self,
		enum ble_device_addr addr_type, uint8_t addr[BLE_ADDR_LEN])
{
	int rc = 0;

	if (!self->onair) {
		self->addr_type = addr_type;
		if (addr) {
			memcpy(self->addr, addr, sizeof(self->addr));
		}

		if ((rc = initialize(self)) == 0) {
			self->onair = true;
		}
	}

	return rc;
}

static int disable_device(struct ble *self)
{
	if (!nrf_sdh_is_enabled()) {
		goto out;
	}

out:
#if 0 /* TODO: implement disable functionality */
	self->onair = false;
#endif
	return 0;
}

struct ble *ble_create(int id)
{
	if (id != 0) {
		return NULL;
	}

	static_instance = (struct ble) {
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

	return &static_instance;
}

void ble_destroy(struct ble *iface)
{
	memset(iface, 0, sizeof(*iface));
}
