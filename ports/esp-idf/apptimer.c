#include "libmcu/apptimer.h"
#include <stdlib.h>
#include "esp_timer.h"

#define GET_STATIC_FLAG(p)	((uintptr_t)(p) & 1UL)
#define SET_STATIC_FLAG(p)	{ \
	uintptr_t t = (uintptr_t)(p) | 1UL; \
	(p) = (typeof(p))t; }
#define UNMASK_STATIC_FLAG(p)	((typeof(p))(((uintptr_t)(p)) & ~1UL))

struct apptimer {
	esp_timer_handle_t handle;
	uint64_t timeout_us;
	esp_err_t (*start)(esp_timer_handle_t handle, uint64_t usec);
};

static bool is_created_static(struct apptimer *timer)
{
	return GET_STATIC_FLAG(timer->handle);
}

static bool create_core(struct apptimer *timer, uint32_t timeout_ms, bool repeat,
		void (*callback)(void *param))
{
	timer->timeout_us = (uint64_t)timeout_ms * 1000;

	if (esp_timer_create(&(const esp_timer_create_args_t) {
				.callback = callback, },
			&timer->handle) != ESP_OK) {
		return false;
	}

	if (repeat) {
		timer->start = esp_timer_start_periodic;
	} else {
		timer->start = esp_timer_start_once;
	}

	return true;
}

int apptimer_create_static(apptimer_t timer, uint32_t timeout_ms, bool repeat,
		void (*callback)(void *param))
{
	struct apptimer *p = (struct apptimer *)timer;

	if (!p) {
		return APPTIMER_INVALID_PARAM;
	}
	if (!create_core(p, timeout_ms, repeat, callback)) {
		return APPTIMER_ERROR;
	}
	SET_STATIC_FLAG(p->handle);

	return APPTIMER_SUCCESS;
}

apptimer_t apptimer_create(uint32_t timeout_ms, bool repeat,
		void (*callback)(void *param))
{
	struct apptimer *p;

	if ((p = (struct apptimer *)calloc(1, sizeof(*p))) == NULL) {
		return NULL;
	}
	if (!create_core(p, timeout_ms, repeat, callback)) {
		free(p);
		return NULL;
	}

	return (apptimer_t)p;
}

int apptimer_destroy(apptimer_t timer)
{
	struct apptimer *p = (struct apptimer *)timer;
	apptimer_error_t rc =
		esp_timer_delete(UNMASK_STATIC_FLAG(p->handle)) == ESP_OK?
		APPTIMER_SUCCESS : APPTIMER_ERROR;
	if (!is_created_static(p)) {
		free(p);
	}
	return rc;
}

int apptimer_start(apptimer_t timer)
{
	struct apptimer *p = (struct apptimer *)timer;
	return p->start(UNMASK_STATIC_FLAG(p->handle), p->timeout_us) == ESP_OK?
		APPTIMER_SUCCESS : APPTIMER_ERROR;
}

int apptimer_stop(apptimer_t timer)
{
	struct apptimer *p = (struct apptimer *)timer;
	return esp_timer_stop(UNMASK_STATIC_FLAG(p->handle)) == ESP_OK?
		APPTIMER_SUCCESS : APPTIMER_ERROR;
}

int apptimer_init(void)
{
	// alread initialized at startup, in do_core_init()
	return esp_timer_init() == ESP_OK?
		APPTIMER_SUCCESS : APPTIMER_ERROR;
}

int apptimer_deinit(void)
{
	return esp_timer_deinit() == ESP_OK?
		APPTIMER_SUCCESS : APPTIMER_ERROR;
}
