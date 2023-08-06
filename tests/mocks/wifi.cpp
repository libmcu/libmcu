#include "CppUTestExt/MockSupport.h"
#include "libmcu/wifi.h"

static struct wifi *iface;
static wifi_event_callback_t callback;
static void *callback_ctx;

int wifi_enable(struct wifi *self) {
	iface = self;
	return mock().actualCall(__func__).returnIntValue();
}

int wifi_disable(struct wifi *self) {
	return mock().actualCall(__func__).returnIntValue();
}

int wifi_connect(struct wifi *self, const struct wifi_conn_param *param) {
	int rc = mock().actualCall(__func__).returnIntValue();
	if (callback) {
		(*callback)(iface, WIFI_EVT_CONNECTED, 0, callback_ctx);
	}
	return rc;
}

int wifi_disconnect(struct wifi *self) {
	return mock().actualCall(__func__).returnIntValue();
}

int wifi_scan(struct wifi *self) {
	return mock().actualCall(__func__).returnIntValue();
}

int wifi_register_event_callback(struct wifi *self, enum wifi_event event_type, const wifi_event_callback_t cb, void *user_ctx) {
	callback = cb;
	callback_ctx = user_ctx;
	return mock().actualCall(__func__).returnIntValue();
}
