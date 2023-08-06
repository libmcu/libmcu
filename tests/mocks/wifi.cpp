#include "CppUTestExt/MockSupport.h"
#include "libmcu/wifi.h"
#include "wifi.h"

int wifi_enable(struct wifi *self) {
	return mock().actualCall(__func__).returnIntValue();
}

int wifi_disable(struct wifi *self) {
	return mock().actualCall(__func__).returnIntValue();
}

int wifi_connect(struct wifi *self, const struct wifi_conn_param *param) {
	return mock().actualCall(__func__).returnIntValue();
}

int wifi_disconnect(struct wifi *self) {
	return mock().actualCall(__func__).returnIntValue();
}

int wifi_scan(struct wifi *self) {
	return mock().actualCall(__func__).returnIntValue();
}

int wifi_register_event_callback(struct wifi *self, enum wifi_event event_type, const wifi_event_callback_t cb, void *user_ctx) {
	return mock().actualCall(__func__).returnIntValue();
}
