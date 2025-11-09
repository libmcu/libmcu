#include "libmcu/wdt.h"

struct lm_wdt {
	int dummy;
};

struct lm_wdt *lm_wdt_new(const char *name, const uint32_t period_ms,
		lm_wdt_timeout_cb_t cb, void *cb_ctx) {
	(void)name;
	(void)period_ms;
	(void)cb;
	(void)cb_ctx;
	static struct lm_wdt wdt;
	return &wdt;
}

void lm_wdt_delete(struct lm_wdt *self) {
	(void)self;
}

int lm_wdt_start(void) {
	return 0;
}

void lm_wdt_stop(void) {
}

int lm_wdt_step(uint32_t *next_deadline_ms) {
	(void)next_deadline_ms;
	return 0;
}

uint32_t lm_wdt_get_time_since_last_feed(const struct lm_wdt *self) {
	(void)self;
	return 0;
}

uint32_t lm_wdt_get_period(const struct lm_wdt *self) {
	(void)self;
	return 0;
}

void lm_wdt_foreach(lm_wdt_foreach_cb_t cb, void *cb_ctx) {
	(void)cb;
	(void)cb_ctx;
}

const char *lm_wdt_name(const struct lm_wdt *self) {
	(void)self;
	return "stub_wdt";
}

bool lm_wdt_is_enabled(const struct lm_wdt *self) {
	(void)self;
	return false;
}

int lm_wdt_enable(struct lm_wdt *self) {
	(void)self;
	return 0;
}

int lm_wdt_disable(struct lm_wdt *self) {
	(void)self;
	return 0;
}

int lm_wdt_register_timeout_cb(lm_wdt_timeout_cb_t cb, void *cb_ctx) {
	(void)cb;
	(void)cb_ctx;
	return 0;
}

int lm_wdt_feed(struct lm_wdt *self) {
	(void)self;
	return 0;
}

int lm_wdt_init(lm_wdt_periodic_cb_t cb, void *cb_ctx, bool threaded)
{
	(void)cb;
	(void)cb_ctx;
	(void)threaded;
	return 0;
}

void lm_wdt_deinit(void) {
}
