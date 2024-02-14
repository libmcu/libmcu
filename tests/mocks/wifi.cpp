#include "CppUTestExt/MockSupport.h"
#include "libmcu/wifi.h"

struct wifi {
	struct wifi_api api;
};

static struct wifi *iface;
static wifi_event_callback_t callback;
static void *callback_ctx;

static int do_connect(struct wifi *self, const struct wifi_conn_param *param) {
	int rc = mock().actualCall("wifi_connect").returnIntValue();
	enum wifi_event evt = rc? WIFI_EVT_DISCONNECTED : WIFI_EVT_CONNECTED;
	if (callback) {
		(*callback)(iface, evt, 0, callback_ctx);
	}
	return rc;
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
