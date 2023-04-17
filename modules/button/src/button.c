/*
 * SPDX-FileCopyrightText: 2021 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/button.h"
#include "libmcu/button_overrides.h"
#include <stdbool.h>
#include <string.h>
#include "libmcu/compiler.h"
#include "libmcu/assert.h"

LIBMCU_STATIC_ASSERT(BUTTON_MAX < 8*sizeof(unsigned int),
		"BUTTON_MAX must be less than bitmap data type size.");
LIBMCU_STATIC_ASSERT(BUTTON_MIN_PRESS_TIME_MS > BUTTON_SAMPLING_PERIOD_MS,
		"The sampling period time must be less than press hold time.");

#define MIN_PRESSED_HISTORY			\
	(BUTTON_MIN_PRESS_TIME_MS / BUTTON_SAMPLING_PERIOD_MS)
#define HISTORY_MASK				\
	((1U << (MIN_PRESSED_HISTORY + 1)) - 1) /* 0b1111111 */
LIBMCU_STATIC_ASSERT(MIN_PRESSED_HISTORY < (8*sizeof(unsigned int) - 2),
		"The history pattern must be within the data type size.");

typedef enum {
	BUTTON_STATE_UNKNOWN		= 0x01U,
	BUTTON_STATE_PRESSED		= 0x02U,
	BUTTON_STATE_RELEASED		= 0x04U,
	BUTTON_STATE_DOWN		= 0x08U,
	BUTTON_STATE_UP			= 0x10U,
	BUTTON_STATE_DEBOUNCING		= 0x20U,
	BUTTON_STATE_INACTIVATED	= 0x40U,
} button_state_t;

struct button_meta {
	struct button data;
	button_handler_t handler;
	int (*get_state)(void);
	bool pressed;
	bool active;
	void *user_ctx;
};

static struct {
	unsigned long (*get_time_ms)(void);

	struct button_meta buttons[BUTTON_MAX];
} m;

static void update_history(struct button_meta *btn)
{
	unsigned int history = ACCESS_ONCE(btn->data.history);
	history <<= 1;
	history |= (unsigned int)(btn->get_state() & 1);
	btn->data.history = history;
}

static unsigned int get_history(const struct button_meta *btn)
{
	return btn->data.history & HISTORY_MASK;
}

static bool is_button_pressed(const struct button_meta *btn)
{
	unsigned int expected = (1U << MIN_PRESSED_HISTORY) - 1; /* 0b0111111 */
	return get_history(btn) == expected;
}

static bool is_button_released(const struct button_meta *btn)
{
	unsigned int expected = 1U << MIN_PRESSED_HISTORY; /* 0b1000000 */
	return get_history(btn) == expected;
}

static bool is_button_up(const struct button_meta *btn)
{
	return get_history(btn) == 0;
}

static bool is_button_down(const struct button_meta *btn)
{
	return get_history(btn) == HISTORY_MASK;
}

static struct button_meta *get_unused_button(void)
{
	for (int i = 0; i < BUTTON_MAX; i++) {
		if (!m.buttons[i].active) {
			return &m.buttons[i];
		}
	}

	return NULL;
}

static bool is_click_window_closed(const struct button_meta *btn, unsigned long t)
{
	return (t - btn->data.time_released) >= BUTTON_CLICK_WINDOW_MS;
}

static void do_pressed(struct button_meta *btn, unsigned long t)
{
	if (btn->pressed) {
		return;
	}

	btn->data.time_pressed = t;
	btn->pressed = true;
	if (btn->handler) {
		btn->handler(BUTTON_EVT_PRESSED, &btn->data, btn->user_ctx);
	}
}

static void do_released(struct button_meta *btn, unsigned long t)
{
	if (!btn->pressed) {
		return;
	}

	btn->data.click++;

	btn->data.time_released = t;
	btn->pressed = false;

	if (btn->handler) {
		btn->handler(BUTTON_EVT_RELEASED, &btn->data, btn->user_ctx);
		btn->handler(BUTTON_EVT_CLICK, &btn->data, btn->user_ctx);
	}

	btn->data.time_repeat = 0;
}

static void do_holding(struct button_meta *btn, unsigned long t)
{
	bool notify = false;

	if (btn->data.time_repeat) {
		if ((t - btn->data.time_repeat) >= BUTTON_REPEAT_RATE_MS) {
			notify = true;
		}
	} else {
		if ((t - btn->data.time_pressed) >= BUTTON_REPEAT_DELAY_MS) {
			notify = true;
		}
	}

	if (notify && btn->handler) {
		btn->data.time_repeat = t;
		btn->handler(BUTTON_EVT_HOLDING, &btn->data, btn->user_ctx);
	}
}

static button_state_t scan_button(struct button_meta *btn, unsigned long t)
{
	if (!btn->active) {
		return BUTTON_STATE_INACTIVATED;
	}

	update_history(btn);

	if (is_button_pressed(btn)) {
		do_pressed(btn, t);
		return BUTTON_STATE_PRESSED;
	} else if (is_button_released(btn)) {
		do_released(btn, t);
		return BUTTON_STATE_RELEASED;
	} else if (is_button_down(btn)) {
		do_holding(btn, t);
		return BUTTON_STATE_DOWN;
	} else if (is_button_up(btn)) {
		return BUTTON_STATE_UP;
	} else if (get_history(btn)) {
		return BUTTON_STATE_DEBOUNCING;
	}

	return BUTTON_STATE_UNKNOWN;
}

static button_rc_t scan_all(unsigned long t)
{
	bool keep_scanning = false;

	for (int i = 0; i < BUTTON_MAX; i++) {
		struct button_meta *btn = &m.buttons[i];
		unsigned int activity_mask = BUTTON_STATE_PRESSED |
			BUTTON_STATE_DOWN | BUTTON_STATE_DEBOUNCING;
		button_state_t state = scan_button(btn, t);

		if (state == BUTTON_STATE_INACTIVATED) {
			continue;
		} else if (state & activity_mask) {
			keep_scanning = true;
		} else { /* button is up */
			if (is_click_window_closed(btn, t)) {
				btn->data.click = 0;
			} else if (btn->data.click > 0) {
				keep_scanning = true;
			}
		}
	}

	if (keep_scanning) {
		return BUTTON_SCANNING;
	}

	return BUTTON_NO_ACTIVITY;
}

static button_rc_t do_step(void)
{
	/* NOTE: Time counter wraparound may add latency by
	 * BUTTON_SAMPLING_PERIOD_MS */
	static unsigned long t0;
	unsigned long t = m.get_time_ms();
	button_rc_t rc = BUTTON_BUSY;

	if ((t - t0) < BUTTON_SAMPLING_PERIOD_MS) {
		return BUTTON_BUSY;
	}

	rc = scan_all(t);
	t0 = t;

	return rc;
}

button_rc_t button_step(void)
{
	button_lock();
	button_rc_t rc = do_step();
	button_unlock();

	return rc;
}

const void *button_register(int (*get_button_state)(void),
		button_handler_t handler, void *ctx)
{
	if (get_button_state == NULL) {
		return NULL;
	}

	struct button_meta *btn = NULL;

	button_lock();

	if ((btn = get_unused_button()) == NULL) {
		goto out;
	}

	btn->handler = handler;
	btn->get_state = get_button_state;
	btn->pressed = false;
	memset(&btn->data, 0, sizeof(btn->data));
	btn->active = true;
	btn->user_ctx = ctx;
out:
	button_unlock();

	return btn;
}

void button_init(unsigned long (*get_time_ms)(void))
{
	assert(get_time_ms != NULL);
	m.get_time_ms = get_time_ms;

	memset(m.buttons, 0, sizeof(m.buttons));
}
