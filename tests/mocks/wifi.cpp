#include "CppUTestExt/MockSupport.h"
#include "libmcu/wifi.h"

void fake_wifi_trigger_callback(enum wifi_event evt, const void *data);

struct wifi {
	struct wifi_api api;
};

static struct wifi *iface;
static wifi_event_callback_t callback;
static void *callback_ctx;

static int do_connect(struct wifi *self, const struct wifi_conn_param *param) {
	return mock().actualCall("wifi_connect")
		.withMemoryBufferParameter("ssid", (const uint8_t *)param->ssid, param->ssid_len)
		.withMemoryBufferParameter("pass", (const uint8_t *)param->psk, param->psk_len)
		.returnIntValue();
}

static int do_disconnect(struct wifi *self) {
	return mock().actualCall("wifi_disconnect").returnIntValue();
}

static int do_scan(struct wifi *self) {
	return mock().actualCall("wifi_scan").returnIntValue();
}

static int do_enable(struct wifi *self) {
	iface = self;
	return mock().actualCall("wifi_enable").returnIntValue();
}

static int do_disable(struct wifi *self) {
	return mock().actualCall("wifi_disable").returnIntValue();
}

static int do_register_event_callback(struct wifi *self,
		enum wifi_event event_type,
		const wifi_event_callback_t cb, void *user_ctx) {
	callback = cb;
	callback_ctx = user_ctx;
	return mock().actualCall("wifi_register_event_callback").returnIntValue();
}

struct wifi *wifi_create(int id) {
	static struct wifi iface = {
		.api = {
			.connect = do_connect,
			.disconnect = do_disconnect,
			.scan = do_scan,
			.enable = do_enable,
			.disable = do_disable,
			.register_event_callback = do_register_event_callback,
		},
	};
	return &iface;
}

int wifi_delete(struct wifi *self) {
	(void)self;
	return 0;
}

void fake_wifi_trigger_callback(enum wifi_event evt, const void *data)
{
	if (callback) {
		(*callback)(iface, evt, data, callback_ctx);
	}
}
