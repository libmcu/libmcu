#include "libmcu/wdt.h"

struct wdt {
	int dummy;
};

struct wdt *wdt_new(const char *name, const uint32_t period_ms,
		wdt_timeout_cb_t cb, void *cb_ctx) {
	(void)name;
	(void)period_ms;
	(void)cb;
	(void)cb_ctx;
	static struct wdt wdt;
	return &wdt;
}

void wdt_delete(struct wdt *self) {
	(void)self;
}

int wdt_start(void) {
	return 0;
}

void wdt_stop(void) {
}

int wdt_step(uint32_t *next_deadline_ms) {
	(void)next_deadline_ms;
	return 0;
}

uint32_t wdt_get_time_since_last_feed(const struct wdt *self) {
	(void)self;
	return 0;
}

uint32_t wdt_get_period(const struct wdt *self) {
	(void)self;
	return 0;
}

void wdt_foreach(wdt_foreach_cb_t cb, void *cb_ctx) {
	(void)cb;
	(void)cb_ctx;
}

const char *wdt_name(const struct wdt *self) {
	(void)self;
	return "stub_wdt";
}

bool wdt_is_enabled(const struct wdt *self) {
	(void)self;
	return false;
}

int wdt_enable(struct wdt *self) {
	(void)self;
	return 0;
}

int wdt_disable(struct wdt *self) {
	(void)self;
	return 0;
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

int wdt_init(wdt_periodic_cb_t cb, void *cb_ctx, bool threaded)
{
	(void)cb;
	(void)cb_ctx;
	(void)threaded;
	return 0;
}

void wdt_deinit(void) {
}
