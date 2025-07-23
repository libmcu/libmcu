/*
 * SPDX-FileCopyrightText: 2022 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_WIFI_H
#define LIBMCU_WIFI_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include "libmcu/ip.h"

#if !defined(WIFI_SSID_MAX_LEN)
#define WIFI_SSID_MAX_LEN		32U
#endif
#if !defined(WIFI_MAC_ADDR_LEN)
#define WIFI_MAC_ADDR_LEN		6U
#endif

enum wifi_event {
	WIFI_EVT_UNKNOWN,
	WIFI_EVT_CONNECTED,
	WIFI_EVT_DISCONNECTED,
	WIFI_EVT_SCAN_RESULT,
	WIFI_EVT_SCAN_DONE,
	WIFI_EVT_STARTED,
	WIFI_EVT_STOPPED,
	WIFI_EVT_ANY,
};

enum wifi_frequency_band {
	WIFI_FREQ_2_4_GHZ,
	WIFI_FREQ_5_GHZ,
	WIFI_FREQ_6_GHZ,
};

enum wifi_security {
	WIFI_SEC_TYPE_UNKNOWN,
	WIFI_SEC_TYPE_NONE,
	WIFI_SEC_TYPE_WEP,
	WIFI_SEC_TYPE_PSK,
	WIFI_SEC_TYPE_PSK_SHA256,
	WIFI_SEC_TYPE_PSK_SAE,
};

enum wifi_mfp {
	WIFI_MFP_OPTIONAL,
	WIFI_MFP_REQUIRED,
	WIFI_MFP_DISABLED,
};

struct wifi_conn_param {
	const uint8_t *ssid;
	uint8_t ssid_len;
	const uint8_t *psk;
	uint8_t psk_len;
	const uint8_t *sae;
	uint8_t sae_len;

	enum wifi_frequency_band band;
	uint8_t channel;
	enum wifi_security security;
	enum wifi_mfp mfp;

	int timeout_ms;
};

struct wifi_iface_info {
	uint8_t ssid[WIFI_SSID_MAX_LEN];
	uint8_t ssid_len;

	uint8_t mac[WIFI_MAC_ADDR_LEN];
	uint8_t mac_len;

	uint8_t channel;
	int8_t rssi;
	enum wifi_security security;

	lm_ip_info_t ip_info;
};

struct wifi_scan_result {
	uint8_t ssid[WIFI_SSID_MAX_LEN];
	uint8_t ssid_len;

	enum wifi_frequency_band band;
	uint8_t channel;
	enum wifi_security security;
	enum wifi_mfp mfp;
	int8_t rssi;

	uint8_t mac[WIFI_MAC_ADDR_LEN];
	uint8_t mac_len;
};

struct wifi;

typedef void (*wifi_event_callback_t)(const struct wifi *self,
		enum wifi_event evt, const void *data, void *ctx);

struct wifi_api {
	int (*connect)(struct wifi *self, const struct wifi_conn_param *param);
	int (*disconnect)(struct wifi *self);
	int (*scan)(struct wifi *self);
	int (*enable)(struct wifi *self);
	int (*disable)(struct wifi *self);
	int (*get_status)(struct wifi *self, struct wifi_iface_info *info);
	int (*register_event_callback)(struct wifi *self,
			enum wifi_event event_type,
			const wifi_event_callback_t cb, void *cb_ctx);
};

struct wifi_event_callback {
	wifi_event_callback_t func;
	enum wifi_event event_type;
	void *user_ctx;
};

static inline int wifi_connect(struct wifi *self,
		const struct wifi_conn_param *param) {
	return ((struct wifi_api *)self)->connect(self, param);
}

static inline int wifi_disconnect(struct wifi *self) {
	return ((struct wifi_api *)self)->disconnect(self);
}

static inline int wifi_scan(struct wifi *self) {
	return ((struct wifi_api *)self)->scan(self);
}

static inline int wifi_enable(struct wifi *self) {
	return ((struct wifi_api *)self)->enable(self);
}

static inline int wifi_disable(struct wifi *self) {
	return ((struct wifi_api *)self)->disable(self);
}

static inline int wifi_get_status(struct wifi *self,
		struct wifi_iface_info *info) {
	return ((struct wifi_api *)self)->get_status(self, info);
}

static inline int wifi_register_event_callback(struct wifi *self,
		enum wifi_event event_type,
		const wifi_event_callback_t cb, void *cb_ctx) {
	return ((struct wifi_api *)self)->register_event_callback(self,
			event_type, cb, cb_ctx);
}

struct wifi *wifi_create(int id);
int wifi_delete(struct wifi *self);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_WIFI_H */
