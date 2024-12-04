#include "libmcu/wdt.h"

struct wdt {
	int dummy;
};

struct wdt *wdt_new(const uint32_t period_ms, wdt_timeout_cb_t cb, void *cb_ctx) {
	static struct wdt wdt;
	return &wdt;
}

void wdt_delete(struct wdt *self) {
	(void)self;
}

int wdt_register_timeout_cb(wdt_timeout_cb_t cb, void *cb_ctx) {
	(void)cb;
	(void)cb_ctx;
	return 0;
}

int wdt_feed(struct wdt *self) {
	(void)self;
	return 0;
}

int wdt_init(void) {
	return 0;
}

void wdt_deinit(void) {
}
