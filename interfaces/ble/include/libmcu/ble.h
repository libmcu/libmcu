/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_BLE_H
#define LIBMCU_BLE_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>

#if !defined(BLE_DEFAULT_DEVICE_NAME)
#define BLE_DEFAULT_DEVICE_NAME			"libmcu"
#endif

#if !defined(BLE_ADV_MAX_PAYLOAD_SIZE)
#define BLE_ADV_MAX_PAYLOAD_SIZE		\
		(37U - 6U/*advertiser address*/ - 2U/*header+length*/)
#endif
#define BLE_TIME_FOREVER			UINT32_MAX
#define BLE_ADDR_LEN				6U
#define BLE_ADV_MIN_INTERVAL_MS			20U
#define BLE_ADV_MAX_INTERVAL_MS			10028U

#define BLE_SERVICE_REQUIRED_MEMORY		(48+24+20) /* bytes */
#define BLE_CHARACTERISTIC_REQUIRED_MEMORY	56 /* bytes */

enum ble_adv_mode {
	BLE_ADV_IND,         /**< connectable     scannable     undirected */
	BLE_ADV_DIRECT_IND,  /**< connectable     non-scannable directed */
	BLE_ADV_NONCONN_IND, /**< non-connectable non-scannable undirected */
	BLE_ADV_SCAN_IND,    /**< non-connectable scannable     undirected */
};

enum ble_adv_flag {
	BLE_ADV_FLAG_LE_LIMITED_DISC		= (1 << 0),
	BLE_ADV_FLAG_LE_GENERAL_DISC		= (1 << 1),
	BLE_ADV_FLAG_BR_EDR_NOT_SUPPORT		= (1 << 2),
	BLE_ADV_FLAG_LE_BR_EDR_CONTROLLER	= (1 << 3),
	BLE_ADV_FLAG_LE_BR_EDR_HOST		= (1 << 4),
};

enum ble_adv_data_type {
	BLE_ADV_TYPE_FLAGS			= 0x01,
	BLE_ADV_TYPE_UUID_16BIT_PARTIAL		= 0x02,
	BLE_ADV_TYPE_UUID_16BIT			= 0x03,
	BLE_ADV_TYPE_UUID_32BIT_PARTIAL		= 0x04,
	BLE_ADV_TYPE_UUID_32BIT			= 0x05,
	BLE_ADV_TYPE_UUID_128BIT_PARTIAL	= 0x06,
	BLE_ADV_TYPE_UUID_128BIT		= 0x07,
	BLE_ADV_TYPE_SHORT_LOCAL_NAME		= 0x08,
	BLE_ADV_TYPE_COMPLETE_LOCAL_NAME	= 0x09,
	BLE_ADV_TYPE_TX_POWER_LEVEL		= 0x0A,
	BLE_ADV_TYPE_CLASS_OF_DEVICE		= 0x0D,
	BLE_ADV_TYPE_SERVICE_DATA		= 0x16,
	BLE_ADV_TYPE_PUBLIC_TARGET_ADDRESS	= 0x17,
	BLE_ADV_TYPE_RANDOM_TARGET_ADDRESS	= 0x18,
	BLE_ADV_TYPE_APPEARANCE			= 0x19,
	BLE_ADV_TYPE_ADVERTISING_INTERVAL	= 0x1A,
	BLE_ADV_TYPE_URI			= 0x24,
	BLE_ADV_TYPE_MANUFACTURER_SPECIFIC	= 0xFF,
};

enum ble_gap_evt {
	BLE_GAP_EVT_UNKNOWN,
	BLE_GAP_EVT_READY,
	BLE_GAP_EVT_CONNECTED,
	BLE_GAP_EVT_DISCONNECTED,
	BLE_GAP_EVT_ADV_COMPLETE,
	BLE_GAP_EVT_ADV_SUSPENDED,
	BLE_GAP_EVT_MTU,
	BLE_GAP_EVT_MAX,
};

enum ble_gatt_evt {
	BLE_GATT_EVT_READ_CHR,
	BLE_GATT_EVT_WRITE_CHR,
	BLE_GATT_EVT_READ_DSC,
	BLE_GATT_EVT_WRITE_DSC,
};

enum ble_device_addr {
	BLE_ADDR_PUBLIC,
	BLE_ADDR_STATIC_RPA,
	BLE_ADDR_PRIVATE_RPA,
	BLE_ADDR_PRIVATE_NRPA,
};

enum ble_gatt_op {
	BLE_GATT_OP_READ			= 0x0001,
	BLE_GATT_OP_WRITE			= 0x0002,
	BLE_GATT_OP_NOTIFY			= 0x0004,
	BLE_GATT_OP_INDICATE			= 0x0008,
	BLE_GATT_OP_ENC_READ			= 0x0010,
	BLE_GATT_OP_AUTH_READ			= 0x0020,
	BLE_GATT_OP_AUTHORIZE_READ		= 0x0040,
	BLE_GATT_OP_ENC_WRITE			= 0x0080,
	BLE_GATT_OP_AUTH_WRITE			= 0x0100,
	BLE_GATT_OP_AUTHORIZE_WRITE		= 0x0200,
};

struct ble;
struct ble_gatt_service;

typedef void (*ble_event_callback_t)(struct ble *self,
		uint8_t evt, const void *msg);

struct ble_handler_context {
	uint8_t event_type;
	void *ctx;
};

typedef void (*ble_gatt_characteristic_handler)(struct ble_handler_context *ctx,
		const void *data, uint16_t datasize, void *user_ctx);

struct ble_adv_payload {
	uint8_t payload[BLE_ADV_MAX_PAYLOAD_SIZE];
	uint8_t index;
};

struct ble_gatt_characteristic {
	ble_gatt_characteristic_handler handler;
	void *user_ctx;
	uint16_t op;
};

struct ble_api {
	int (*enable)(struct ble *self, enum ble_device_addr addr_type,
			uint8_t addr[BLE_ADDR_LEN]);
	int (*disable)(struct ble *self);
	enum ble_device_addr (*get_device_address)(struct ble *self,
			uint8_t addr[BLE_ADDR_LEN]);

	void (*register_gap_event_callback)(struct ble *self,
			ble_event_callback_t cb);
	void (*register_gatt_event_callback)(struct ble *self,
			ble_event_callback_t cb);

	int (*adv_init)(struct ble *self, enum ble_adv_mode mode);
	int (*adv_set_interval)(struct ble *self,
			uint16_t min_ms, uint16_t max_ms);
	int (*adv_set_duration)(struct ble *self, uint32_t msec);
	int (*adv_set_payload)(struct ble *self,
			const struct ble_adv_payload *payload);
	int (*adv_set_scan_response)(struct ble *self,
			const struct ble_adv_payload *payload);
	int (*adv_start)(struct ble *self);
	int (*adv_stop)(struct ble *self);

	struct ble_gatt_service *(*gatt_create_service)(void *mem, uint16_t memsize,
			const uint8_t *uuid, uint8_t uuid_len,
			bool primary, uint8_t nr_characteristics);
	const uint16_t *(*gatt_add_characteristic)(struct ble_gatt_service *svc,
			const uint8_t *uuid, uint8_t uuid_len,
			struct ble_gatt_characteristic *chr);
	int (*gatt_register_service)(struct ble *self,
			struct ble_gatt_service *svc);
	int (*gatt_response)(struct ble_handler_context *ctx,
			const void *data, uint16_t datasize);
	int (*gatt_notify)(struct ble *self, const void *attr_handle,
			const void *data, uint16_t datasize);
};

static inline int ble_enable(struct ble *self,
		enum ble_device_addr addr_type, uint8_t addr[BLE_ADDR_LEN])
{
	return ((struct ble_api *)self)->enable(self, addr_type, addr);
}

static inline int ble_disable(struct ble *self)
{
	return ((struct ble_api *)self)->disable(self);
}

static inline enum ble_device_addr ble_get_device_address(struct ble *self,
		uint8_t addr[BLE_ADDR_LEN])
{
	return ((struct ble_api *)self)->get_device_address(self, addr);
}

static inline void ble_register_gap_event_callback(struct ble *self,
						  ble_event_callback_t cb)
{
	((struct ble_api *)self)->register_gap_event_callback(self, cb);
}

static inline void ble_register_gatt_event_callback(struct ble *self,
						  ble_event_callback_t cb)
{
	((struct ble_api *)self)->register_gatt_event_callback(self, cb);
}

static inline int ble_adv_init(struct ble *self, enum ble_adv_mode mode)
{
	return ((struct ble_api *)self)->adv_init(self, mode);
}

static inline int ble_adv_set_interval(struct ble *self,
			uint16_t min_ms, uint16_t max_ms)
{
	return ((struct ble_api *)self)->adv_set_interval(self,
						min_ms, max_ms);
}

static inline int ble_adv_set_duration(struct ble *self, uint32_t msec)
{
	return ((struct ble_api *)self)->adv_set_duration(self, msec);
}

static inline int ble_adv_set_payload(struct ble *self,
			const struct ble_adv_payload *payload)
{
	return ((struct ble_api *)self)->adv_set_payload(self, payload);
}

static inline int ble_adv_set_scan_response(struct ble *self,
			const struct ble_adv_payload *payload)
{
	return ((struct ble_api *)self)->adv_set_scan_response(self,
						payload);
}

static inline int ble_adv_start(struct ble *self)
{
	return ((struct ble_api *)self)->adv_start(self);
}

static inline int ble_adv_stop(struct ble *self)
{
	return ((struct ble_api *)self)->adv_stop(self);
}

static inline struct ble_gatt_service *ble_gatt_create_service(
		struct ble *self, void *mem, uint16_t memsize,
		const uint8_t *uuid, uint8_t uuid_len,
		bool primary, uint8_t nr_chrs)
{
	return ((struct ble_api *)self)->gatt_create_service(mem,
			memsize, uuid, uuid_len, primary, nr_chrs);
}

static inline const uint16_t *ble_gatt_add_characteristic(struct ble *self,
		struct ble_gatt_service *svc,
		const uint8_t *uuid, uint8_t uuid_len,
		struct ble_gatt_characteristic *chr)
{
	return ((struct ble_api *)self)->gatt_add_characteristic(svc,
			uuid, uuid_len, chr);
}

static inline int ble_gatt_register_service(struct ble *self,
		struct ble_gatt_service *svc)
{
	return ((struct ble_api *)self)->gatt_register_service(self, svc);
}

static inline int ble_gatt_response(struct ble *self,
		struct ble_handler_context *ctx,
		const void *data, uint16_t datasize)
{
	return ((struct ble_api *)self)->gatt_response(ctx,
				data, datasize);
}

static inline int ble_gatt_notify(struct ble *self,
		const void *attr_handle, const void *data, uint16_t datasize)
{
	return ((struct ble_api *)self)->gatt_notify(self, attr_handle,
				data, datasize);
}

void ble_adv_payload_init(struct ble_adv_payload *buf);
int ble_adv_payload_add(struct ble_adv_payload *buf, uint8_t type,
			const void *data, uint8_t data_len);

struct ble *ble_create(int id);
void ble_destroy(struct ble *iface);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_BLE_H */
