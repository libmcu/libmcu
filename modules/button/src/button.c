/*
 * SPDX-FileCopyrightText: 2021 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/button.h"
#include "libmcu/button_overrides.h"
#include "libmcu/compiler.h"

#include <string.h>

#if !defined(BUTTON_MAX)
#define BUTTON_MAX				8
#endif
#if !defined(BUTTON_SAMPLING_PERIOD_MS)
#define BUTTON_SAMPLING_PERIOD_MS		10U
#endif
#if !defined(BUTTON_DEBOUNCE_DURATION_MS)
#define BUTTON_DEBOUNCE_DURATION_MS		20U
#endif
#if !defined(BUTTON_REPEAT_DELAY_MS)
#define BUTTON_REPEAT_DELAY_MS			300U
#endif
#if !defined(BUTTON_REPEAT_RATE_MS)
#define BUTTON_REPEAT_RATE_MS			200U
#endif
#if !defined(BUTTON_CLICK_WINDOW_MS)
#define BUTTON_CLICK_WINDOW_MS			500U
#endif
#if !defined(BUTTON_SAMPLING_TIMEOUT_MS)
#define BUTTON_SAMPLING_TIMEOUT_MS		1000U
#endif
static_assert(BUTTON_DEBOUNCE_DURATION_MS > BUTTON_SAMPLING_PERIOD_MS,
		"The sampling period time must be less than press hold time.");

typedef enum {
	ACTION_IDLE				= 0x00U,
	ACTION_PRESSED				= 0x01U,
	ACTION_RELEASED				= 0x02U,
	ACTION_DOWN				= 0x04U,
	ACTION_UP				= 0x08U,
	ACTION_DEBOUNCING			= 0x10U,
} action_t;

typedef uint32_t waveform_t;

struct button_data {
	waveform_t waveform;
	uint32_t time_pressed;
	uint32_t time_released;
	uint32_t time_repeat;
	uint16_t clicks; /**< the number of clicks */
	uint16_t repeats; /**< the number of repeats */
};

struct button {
	struct button_data data;
	struct button_param param;

	button_get_state_func_t get_state;
	void *get_state_ctx;
	button_callback_t callback;
	void *callback_ctx;

	uint32_t timestamp;
	button_state_t state;

	bool allocated;
	bool active;
	bool pressed;
};

static struct button *new_button(struct button *btns)
{
	for (int i = 0; i < BUTTON_MAX; i++) {
		struct button *p = &btns[i];
		if (!p->allocated) {
			p->allocated = true;
			return p;
		}
	}

	return NULL;
}

static void free_button(struct button *btn)
{
	memset(btn, 0, sizeof(*btn));
}

static void get_default_param(struct button_param *param)
{
	*param = (struct button_param) {
		.sampling_interval_ms = BUTTON_SAMPLING_PERIOD_MS,
		.min_press_time_ms = BUTTON_DEBOUNCE_DURATION_MS,
		.repeat_delay_ms = BUTTON_REPEAT_DELAY_MS,
		.repeat_rate_ms = BUTTON_REPEAT_RATE_MS,
		.click_window_ms = BUTTON_CLICK_WINDOW_MS,
		.max_sampling_interval_ms = BUTTON_SAMPLING_TIMEOUT_MS,
	};
}

static uint16_t get_min_pressed_pulse_count(const struct button *btn)
{
	return btn->param.min_press_time_ms / btn->param.sampling_interval_ms;
}

static waveform_t get_min_pressed_waveform(const struct button *btn)
{
	return 1U << get_min_pressed_pulse_count(btn);
}

static waveform_t get_waveform_debouncing_mask(const struct button *btn)
{
	return (1U << get_min_pressed_pulse_count(btn)) - 1;
}

static waveform_t get_waveform_mask(const struct button *btn)
{
	return (1U << (get_min_pressed_pulse_count(btn) + 1)) - 1;
}

static waveform_t get_waveform(const struct button *btn)
{
	return btn->data.waveform & get_waveform_mask(btn);
}

static void update_waveform(waveform_t *waveform, const button_level_t pressed)
{
	*waveform <<= 1;
	*waveform |= pressed;
}

static bool is_param_ok(const struct button_param *param,
		const uint16_t min_pulse_count)
{
	if (!param->sampling_interval_ms) {
		return false;
	}

	if (param->min_press_time_ms < param->sampling_interval_ms) {
		return false;
	}

	if (param->max_sampling_interval_ms < param->min_press_time_ms ||
			param->max_sampling_interval_ms < param->repeat_delay_ms ||
			param->max_sampling_interval_ms < param->repeat_rate_ms ||
			param->max_sampling_interval_ms < param->click_window_ms) {
		return false;
	}

	if (min_pulse_count >= (uint16_t)(sizeof(waveform_t) * 8 - 2)) {
		return false;
	}

	return true;
}

static bool is_button_pressed(const struct button *btn)
{
	if (btn->pressed) { /* already pressed */
		return false;
	}

	/* 0b0111111 */
	const waveform_t expected = get_min_pressed_waveform(btn) - 1;
	const waveform_t mask = get_waveform_debouncing_mask(btn);
	return (get_waveform(btn) & mask) == expected;
}

static bool is_button_released(const struct button *btn)
{
	if (!btn->pressed) { /* already released */
		return false;
	}

	/* 0b1000000 */
	const waveform_t expected = get_min_pressed_waveform(btn);
	return get_waveform(btn) == expected;
}

static bool is_button_up(const struct button *btn)
{
	const waveform_t mask = get_waveform_debouncing_mask(btn);
	return (get_waveform(btn) & mask) == 0;
}

static bool is_button_down(const struct button *btn)
{
	const waveform_t mask = get_waveform_debouncing_mask(btn);
	return (get_waveform(btn) & mask) == mask;
}

static bool is_click_window_closed(const struct button *btn,
		const uint32_t time_ms)
{
	if (!btn->param.click_window_ms) {
		return true;
	}
	return time_ms - btn->data.time_released >= btn->param.click_window_ms;
}

static waveform_t update_state(struct button *btn, const uint32_t pulses)
{
	/* If the elapsed time is greater than the sampling interval, update
	 * historical waveform to the current state as well. */
	for (uint32_t i = 0; i < pulses; i++) {
		const button_level_t level =
			(*btn->get_state)(btn->get_state_ctx);
		update_waveform(&btn->data.waveform, level);
	}

	return get_waveform(btn);
}

static bool process_pressed(struct button *btn, const uint32_t time_ms)
{
	btn->data.time_pressed = time_ms;
	btn->data.clicks++;
	btn->pressed = true;
	return true;
}

static bool process_released(struct button *btn, const uint32_t time_ms)
{
	btn->data.time_released = time_ms;
	btn->pressed = false;
	btn->data.time_repeat = 0;
	btn->data.repeats = 0;
	return true;
}

static bool process_holding(struct button *btn, const uint32_t time_ms)
{
	bool state_updated = false;

	if (!btn->param.repeat_delay_ms) {
		goto out;
	}

	if (btn->data.time_repeat) {
		if (!btn->param.repeat_rate_ms) {
			goto out;
		} else if ((time_ms - btn->data.time_repeat)
				>= btn->param.repeat_rate_ms) {
			state_updated = true;
		}
	} else {
		if ((time_ms - btn->data.time_pressed)
				>= btn->param.repeat_delay_ms) {
			state_updated = true;
		}
	}

	if (state_updated) {
		btn->data.time_repeat = time_ms;
		btn->data.repeats++;
	}

out:
	return state_updated;
}

static button_state_t process_button(struct button *btn, const uint32_t time_ms)
{
	const uint32_t elapsed_ms = time_ms - btn->timestamp;
	uint32_t pulses = elapsed_ms / btn->param.sampling_interval_ms;
	button_state_t state = BUTTON_STATE_UNKNOWN;

	if (!pulses) {
		goto out;
	} else if (elapsed_ms > btn->param.max_sampling_interval_ms) {
		/* synchronize the timestamp as it's been too long since
		 * the last update. The button state is assumed to be the same
		 * as before. */
		btn->timestamp = time_ms;
		pulses = 1;
	}

	const waveform_t waveform = update_state(btn, pulses);
	const uint32_t activity_mask = ACTION_PRESSED | ACTION_DOWN
		| ACTION_DEBOUNCING;
	action_t action = ACTION_IDLE;

	if (is_button_pressed(btn)) {
		action = ACTION_PRESSED;
		state = BUTTON_STATE_PRESSED;
		process_pressed(btn, time_ms);
	} else if (is_button_released(btn)) {
		action = ACTION_RELEASED;
		state = BUTTON_STATE_RELEASED;
		process_released(btn, time_ms);
	} else if (is_button_down(btn)) {
		action = ACTION_DOWN;
		if (process_holding(btn, time_ms)) {
			state = BUTTON_STATE_HOLDING;
		}
	} else if (is_button_up(btn)) {
		action = ACTION_UP;
	} else if (waveform) {
		action = ACTION_DEBOUNCING;
	}

	if (!(action & activity_mask) && is_click_window_closed(btn, time_ms)) {
		btn->data.clicks = 0;
	}

	if (state != BUTTON_STATE_UNKNOWN) {
		btn->state = state;
	}

	btn->timestamp = time_ms;
out:
	return state;
}

static button_error_t do_step(struct button *btn, const uint32_t time_ms)
{
	const button_state_t state = process_button(btn, time_ms);

	if (state != BUTTON_STATE_UNKNOWN && btn->callback) {
		(*btn->callback)(btn, state, btn->data.clicks,
				btn->data.repeats, btn->callback_ctx);
	}

	return BUTTON_ERROR_NONE;
}

button_error_t button_step(struct button *btn, const uint32_t time_ms)
{
	if (btn == NULL) {
		return BUTTON_ERROR_INVALID_PARAM;
	}
	if (!btn->active) {
		return BUTTON_ERROR_DISABLED;
	}

	return do_step(btn, time_ms);
}

button_error_t button_step_delta(struct button *btn, const uint32_t delta_ms)
{
	if (btn == NULL) {
		return BUTTON_ERROR_INVALID_PARAM;
	}
	if (!btn->active) {
		return BUTTON_ERROR_DISABLED;
	}

	const uint32_t time_ms = btn->timestamp + delta_ms;

	return do_step(btn, time_ms);
}

button_error_t button_set_param(struct button *btn,
		const struct button_param *param)
{
	struct button_param copy;

	if (btn == NULL || param == NULL) {
		return BUTTON_ERROR_INVALID_PARAM;
	}

	memcpy(&copy, param, sizeof(copy));

	if (!copy.max_sampling_interval_ms) {
		copy.max_sampling_interval_ms = BUTTON_SAMPLING_TIMEOUT_MS;
	}

	if (copy.sampling_interval_ms &&
			is_param_ok(&copy, copy.min_press_time_ms
				/ copy.sampling_interval_ms)) {
		memcpy(&btn->param, &copy, sizeof(copy));
		return BUTTON_ERROR_NONE;
	}

	return BUTTON_ERROR_INCORRECT_PARAM;
}

button_error_t button_get_param(const struct button *btn,
		struct button_param *param)
{
	if (btn == NULL || param == NULL) {
		return BUTTON_ERROR_INVALID_PARAM;
	}

	memcpy(param, &btn->param, sizeof(*param));
	return BUTTON_ERROR_NONE;
}

bool button_busy(const struct button *btn)
{
	return !is_button_up(btn);
}

button_state_t button_state(const struct button *btn)
{
	return btn->state;
}

uint16_t button_clicks(const struct button *btn)
{
	return btn->data.clicks;
}

uint16_t button_repeats(const struct button *btn)
{
	return btn->data.repeats;
}

button_error_t button_enable(struct button *btn)
{
	if (btn == NULL) {
		return BUTTON_ERROR_INVALID_PARAM;
	}

	btn->active = true;
	return BUTTON_ERROR_NONE;
}

button_error_t button_disable(struct button *btn)
{
	if (btn == NULL) {
		return BUTTON_ERROR_INVALID_PARAM;
	}

	btn->active = false;
	return BUTTON_ERROR_NONE;
}

struct button *button_new(button_get_state_func_t f_get, void *f_get_ctx,
		button_callback_t cb, void *cb_ctx)
{
	static struct button btns[BUTTON_MAX];
	struct button *btn = NULL;

	if (f_get == NULL) {
		return NULL;
	}

	button_lock();
	btn = new_button(btns);
	button_unlock();

	if (btn) {
		btn->get_state = f_get;
		btn->get_state_ctx = f_get_ctx;
		btn->callback = cb;
		btn->callback_ctx = cb_ctx;
		get_default_param(&btn->param);
	}

	return btn;
}

void button_delete(struct button *btn)
{
	button_lock();
	free_button(btn);
	button_unlock();
}
