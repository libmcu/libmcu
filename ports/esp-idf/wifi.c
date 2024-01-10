/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/wifi.h"

#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_mac.h"

#if !defined(WIFI_EVENT_CALLBACK_CAP)
#define WIFI_EVENT_CALLBACK_CAPACITY		2U
#endif

enum esp32_state {
	ESP32_STATE_UNKNOWN,
	ESP32_STATE_STARTING,
	ESP32_STATE_STA_STOPPED,
	ESP32_STATE_STA_STARTED,
	ESP32_STATE_STA_CONNECTING,
	ESP32_STATE_STA_CONNECTED,
	ESP32_STATE_AP_STOPPED,
	ESP32_STATE_AP_CONNECTED,
	ESP32_STATE_AP_DISCONNECTED,
};

struct wifi {
	struct wifi_api api;

	struct wifi_event_callback callbacks[WIFI_EVENT_CALLBACK_CAPACITY];

	struct wifi_iface_info status;

	struct {
		uint8_t v4[4];
		uint32_t v6[4];
	} ip;

	esp_event_handler_instance_t wifi_events;
	esp_event_handler_instance_t ip_acquisition_events;

	enum esp32_state state;
};

static void raise_event_with_data(struct wifi *self,
				  enum wifi_event evt, const void *data)
{
	for (unsigned int i = 0; i < WIFI_EVENT_CALLBACK_CAPACITY; i++) {
		struct wifi_event_callback *cb = &self->callbacks[i];

		if (evt == cb->event_type || cb->event_type == WIFI_EVT_ANY) {
			if (cb->func) {
				(*cb->func)(self, evt, data, cb->user_ctx);
			}
		}
	}
}

static void handle_scan_result_core(struct wifi *self,
		uint16_t n, const wifi_ap_record_t *scanned)
{
	struct wifi_scan_result res = { 0, };

	for (int i = 0; i < n; i++) {
		res.ssid_len = strnlen((const char *)scanned[i].ssid,
				WIFI_SSID_MAX_LEN);
		strncpy((char *)res.ssid, (const char *)scanned[i].ssid,
				res.ssid_len);
		res.rssi = scanned[i].rssi;
		res.channel = scanned[i].primary;
		res.security = WIFI_SEC_TYPE_NONE;
		if (scanned[i].authmode == WIFI_AUTH_WEP) {
			res.security = WIFI_SEC_TYPE_WEP;
		} else if (scanned[i].authmode > WIFI_AUTH_OPEN) {
			res.security = WIFI_SEC_TYPE_PSK;
		}
		res.mac_len = WIFI_MAC_ADDR_LEN;
		memcpy(res.mac, scanned[i].bssid, res.mac_len);

		raise_event_with_data(self, WIFI_EVT_SCAN_RESULT, &res);
	}
}

static void handle_scan_result(struct wifi *self)
{
	uint16_t n = 0;
	wifi_ap_record_t *scanned;

	esp_wifi_scan_get_ap_num(&n);
	if (n == 0) {
		goto out;
	}
	if ((scanned = (wifi_ap_record_t *)calloc(n, sizeof(wifi_ap_record_t)))
			== NULL) {
		goto out;
	}

	if (esp_wifi_scan_get_ap_records(&n, scanned) == ESP_OK) {
		handle_scan_result_core(self, n, scanned);
	}

	free(scanned);
out:
	raise_event_with_data(self, WIFI_EVT_SCAN_DONE, 0);
}

static void handle_scan_done(struct wifi *self)
{
	handle_scan_result(self);
}

static void handle_connected_event(struct wifi *self, ip_event_got_ip_t *ip)
{
	self->state = ESP32_STATE_STA_CONNECTED;
	memcpy(&self->ip, &ip->ip_info.ip, sizeof(self->ip));
	raise_event_with_data(self, WIFI_EVT_CONNECTED, 0);
}


/* https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/event-handling.html#event-ids-and-corresponding-data-structures */
static void on_wifi_events(void *arg, esp_event_base_t event_base,
		int32_t event_id, void *event_data)
{
	struct wifi *self = (struct wifi *)arg;

	switch (event_id) {
	case WIFI_EVENT_STA_START:
		self->state = ESP32_STATE_STA_STARTED;
		raise_event_with_data(self, WIFI_EVT_STARTED, 0);
		break;
	case WIFI_EVENT_STA_STOP:
		self->state = ESP32_STATE_STA_STOPPED;
		raise_event_with_data(self, WIFI_EVT_STOPPED, 0);
		break;
	case WIFI_EVENT_STA_CONNECTED:
		/* The connection event will be raised when IP acquired as DHCP
		 * is getting started automatically by netif which is
		 * registered earlier at the initailization. */
		break;
	case WIFI_EVENT_STA_DISCONNECTED:
		self->state = ESP32_STATE_STA_STARTED;
		memset(&self->ip, 0, sizeof(self->ip));
		raise_event_with_data(self, WIFI_EVT_DISCONNECTED, 0);
		break;
	case WIFI_EVENT_SCAN_DONE:
		handle_scan_done(self);
		break;
	case WIFI_EVENT_AP_START:
	case WIFI_EVENT_AP_STOP:
	case WIFI_EVENT_AP_STACONNECTED:
	case WIFI_EVENT_AP_STADISCONNECTED:
	default:
		break;
	}
}

static void on_ip_events(void *arg, esp_event_base_t event_base,
		int32_t event_id, void *event_data)
{
	struct wifi *self = (struct wifi *)arg;

	switch (event_id) {
	case IP_EVENT_STA_GOT_IP:
		handle_connected_event(self, event_data);
		break;
	case IP_EVENT_STA_LOST_IP:
	default:
		break;
	}
}

static bool initialize_wifi_event(struct wifi *self)
{
	esp_err_t res = esp_event_handler_instance_register(WIFI_EVENT,
			ESP_EVENT_ANY_ID,
			&on_wifi_events,
			self,
			&self->wifi_events);
	res |= esp_event_handler_instance_register(IP_EVENT,
			ESP_EVENT_ANY_ID,
			&on_ip_events,
			self,
			&self->ip_acquisition_events);

	return !res;
}

static bool initialize_wifi_iface(struct wifi *self)
{
	wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
	esp_err_t res = esp_netif_init();

	if (esp_netif_create_default_wifi_sta() == NULL) {
		return false;
	}

	res |= esp_wifi_init(&config);
	res |= esp_wifi_set_mode(WIFI_MODE_STA);

	(void)self;
	return !res;
}

static int initialize_wifi(struct wifi *self)
{
	if (!initialize_wifi_event(self)) {
		return -EBUSY;
	}
	if (!initialize_wifi_iface(self)) {
		return -EAGAIN;
	}

	return 0;
}

static int deinitialize_wifi(struct wifi *self)
{
	(void)self;
	return esp_wifi_deinit() == ESP_OK ? 0 : -EBUSY;
}

static int connect_wifi(struct wifi *self, const struct wifi_conn_param *param)
{
	wifi_config_t conf = {
		.sta = {
			.threshold.authmode = WIFI_AUTH_WPA2_PSK,
			.scan_method = WIFI_ALL_CHANNEL_SCAN,
			.sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
			.pmf_cfg = {
				.capable = true,
				.required = false
			},
		},
	};

	if (self->state == ESP32_STATE_STA_CONNECTING ||
			self->state == ESP32_STATE_STA_CONNECTED) {
		return -EISCONN;
	}
	if (sizeof(conf.sta.ssid) < param->ssid_len ||
			sizeof(conf.sta.password) < param->psk_len) {
		return -ERANGE;
	}

	memcpy(conf.sta.ssid, param->ssid, param->ssid_len);
	memcpy(conf.sta.password, param->psk, param->psk_len);

	if (param->security == WIFI_SEC_TYPE_NONE) {
		conf.sta.threshold.authmode = WIFI_AUTH_OPEN;
	} else if (param->security == WIFI_SEC_TYPE_WEP) {
		conf.sta.threshold.authmode = WIFI_AUTH_WEP;
	}

	if (esp_wifi_set_config(WIFI_IF_STA, &conf) != ESP_OK) {
		return -EINVAL;
	}

	self->state = ESP32_STATE_STA_CONNECTING;

	if (esp_wifi_set_mode(WIFI_MODE_STA) != ESP_OK ||
			esp_wifi_connect() != ESP_OK) {
		return -ENOTCONN;
	}

	memcpy(self->status.ssid, param->ssid, param->ssid_len);
	self->status.ssid_len = param->ssid_len;
	self->status.security = param->security;

	return 0;
}

static int disconnect_wifi(struct wifi *self)
{
	if (self->state != ESP32_STATE_STA_CONNECTED &&
			self->state != ESP32_STATE_STA_CONNECTING) {
		return -ENOTCONN;
	}

	if (esp_wifi_disconnect() != ESP_OK) {
		return -EAGAIN;
	}

	return 0;
}

static int scan_wifi(struct wifi *self)
{
	if (esp_wifi_set_mode(WIFI_MODE_STA) != ESP_OK ||
			esp_wifi_scan_start(&(wifi_scan_config_t) {
						.show_hidden = true,
					}, false) != ESP_OK) {
		return -EAGAIN;
	}

	return 0;
}

static int enable_wifi(struct wifi *self)
{
	return esp_wifi_start() == ESP_OK ? 0 : -EAGAIN;
}

static int disable_wifi(struct wifi *self)
{
	return esp_wifi_stop() == ESP_OK ? 0 : -EBUSY;
}

static int get_status(struct wifi *self, struct wifi_iface_info *info)
{
	memcpy(info, &self->status, sizeof(self->status));
	return 0;
}

static int register_event_callback(struct wifi *self,
		enum wifi_event event_type, const wifi_event_callback_t cb,
		void *user_ctx)
{
	struct wifi_event_callback *p = NULL;

	for (unsigned int i = 0; i < WIFI_EVENT_CALLBACK_CAPACITY; i++) {
		if (self->callbacks[i].func == NULL) {
			p = &self->callbacks[i];
			break;
		}
	}

	if (!p) {
		return -ENOBUFS;
	}

	p->func = cb;
	p->event_type = event_type;
	p->user_ctx = user_ctx;

	return 0;
}

struct wifi *wifi_create(int id)
{
	static struct wifi esp_iface = {
		.api = {
			.connect = connect_wifi,
			.disconnect = disconnect_wifi,
			.scan = scan_wifi,
			.enable = enable_wifi,
			.disable = disable_wifi,
			.get_status = get_status,
			.register_event_callback = register_event_callback,
		},
	};

	if (esp_iface.state != ESP32_STATE_UNKNOWN) {
		return NULL;
	}

	esp_iface.state = ESP32_STATE_STARTING;

	if (initialize_wifi(&esp_iface)) {
		deinitialize_wifi(&esp_iface);
		esp_iface.state = ESP32_STATE_UNKNOWN;
		return NULL;
	}

	esp_read_mac(esp_iface.status.mac, ESP_MAC_WIFI_STA);
	esp_iface.status.mac_len = WIFI_MAC_ADDR_LEN;

	return &esp_iface;
}

int wifi_delete(struct wifi *self)
{
	deinitialize_wifi(self);
	self->state = ESP32_STATE_UNKNOWN;
	return 0;
}
