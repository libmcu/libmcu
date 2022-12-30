/*
 * SPDX-FileCopyrightText: 2021 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/button.h"
#include "libmcu/button_overrides.h"
#include <string.h>
#include "libmcu/compiler.h"
#include "libmcu/assert.h"

#if !defined(BUTTON_MAX)
#define BUTTON_MAX				1
#endif
#if !defined(BUTTON_SAMPLING_PERIOD_MS)
#define BUTTON_SAMPLING_PERIOD_MS		10U
#endif
#if !defined(BUTTON_MIN_PRESS_TIME_MS)
#define BUTTON_MIN_PRESS_TIME_MS		60U
#endif
#if !defined(BUTTON_REPEAT_DELAY_MS)
#define BUTTON_REPEAT_DELAY_MS			300U
#endif
#if 0
#if !defined(BUTTON_REPEAT_RATE_MS)
#define BUTTON_REPEAT_RATE_MS			100U
#endif
#endif
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
	BUTTON_STATE_UNKNOWN			= 0,
	BUTTON_STATE_PRESSED,
	BUTTON_STATE_RELEASED,
	BUTTON_STATE_DOWN,
	BUTTON_STATE_UP,
	BUTTON_STATE_INACTIVE = BUTTON_STATE_UP,
} button_state_t;

struct button {
	struct button_data data;
	const struct button_handlers *ops;
	int (*get_state)(void);
	bool pressed;
	bool active;
	bool holding;
};

static struct {
	unsigned int (*get_time_ms)(void);

	struct button buttons[BUTTON_MAX];
} m;

static void update_button(struct button *btn)
{
	unsigned int history = ACCESS_ONCE(btn->data.history);
	history <<= 1;
	history |= (unsigned int)(btn->get_state() & 1);
	btn->data.history = history;
}

static bool is_button_pressed(const struct button *btn)
{
	unsigned int expected = (1U << MIN_PRESSED_HISTORY) - 1; /* 0b0111111 */
	return (btn->data.history & HISTORY_MASK) == expected;
}

static bool is_button_released(const struct button *btn)
{
	unsigned int expected = 1U << MIN_PRESSED_HISTORY; /* 0b1000000 */
	return (btn->data.history & HISTORY_MASK) == expected;
}

static bool is_button_up(const struct button *btn)
{
	return (btn->data.history & HISTORY_MASK) == 0;
}

static bool is_button_down(const struct button *btn)
{
	return (btn->data.history & HISTORY_MASK) == HISTORY_MASK;
}

static struct button *get_unused_button(void)
{
	for (int i = 0; i < BUTTON_MAX; i++) {
		if (!m.buttons[i].active) {
			return &m.buttons[i];
		}
	}

	return NULL;
}

static void do_pressed(struct button *btn, void *context, unsigned int t)
{
	if (btn->pressed) {
		return;
	}

	btn->data.time_pressed = t;
	btn->pressed = true;
	if (btn->ops->pressed) {
		btn->ops->pressed(&btn->data, context);
	}
}

static void do_released(struct button *btn, void *context, unsigned int t)
{
	if (!btn->pressed) {
		return;
	}

	btn->data.time_released = t;
	btn->pressed = false;
	btn->holding = false;
	if (btn->ops->released) {
		btn->ops->released(&btn->data, context);
	}
}

static void do_holding(struct button *btn, void *context, unsigned int t)
{
	if (btn->holding) {
		return;
	}

	if ((t - btn->data.time_pressed) >= BUTTON_REPEAT_DELAY_MS) {
		btn->holding = true;
		if (btn->ops->holding) {
			btn->ops->holding(&btn->data, context);
		}
	}
}

static button_state_t scan_button(struct button *btn, void *context,
		unsigned int t)
{
	if (!btn->active) {
		return BUTTON_STATE_INACTIVE;
	}

	update_button(btn);

	if (is_button_pressed(btn)) {
		do_pressed(btn, context, t);
		return BUTTON_STATE_PRESSED;
	} else if (is_button_released(btn)) {
		do_released(btn, context, t);
		return BUTTON_STATE_RELEASED;
	} else if (is_button_down(btn)) {
		do_holding(btn, context, t);
		return BUTTON_STATE_DOWN;
	} else if (is_button_up(btn)) {
		return BUTTON_STATE_UP;
	}

	return BUTTON_STATE_UNKNOWN;
}

static void scan_all(void *context, unsigned int t)
{
	for (int i = 0; i < BUTTON_MAX; i++) {
		scan_button(&m.buttons[i], context, t);
	}
}

static bool button_poll_internal(void *context)
{
	static unsigned int t0;
	unsigned int t = m.get_time_ms();

	if ((t - t0) < BUTTON_SAMPLING_PERIOD_MS) {
		return false;
	}

	scan_all(context, t);

	t0 = t;

	return true;
}

bool button_poll(void *context)
{
	button_lock();
	bool rc = button_poll_internal(context);
	button_unlock();

	return rc;
}

const void *button_register(const struct button_handlers *handlers,
		int (*get_button_state)(void))
{
	if (handlers == NULL || get_button_state == NULL) {
		return NULL;
	}

	struct button *btn = NULL;

	button_lock();

	if ((btn = get_unused_button()) == NULL) {
		goto out;
	}

	btn->ops = handlers;
	btn->get_state = get_button_state;
	btn->pressed = false;
	btn->holding = false;
	memset(&btn->data, 0, sizeof(btn->data));
	btn->active = true;
out:
	button_unlock();

	return btn;
}

bool button_is_pressed(const void *handle)
{
	button_lock();
	bool pressed = !is_button_up((const struct button *)handle);
	button_unlock();

	return pressed;
}

void button_init(unsigned int (*get_time_ms)(void))
{
	assert(get_time_ms != NULL);
	m.get_time_ms = get_time_ms;

	memset(m.buttons, 0, sizeof(m.buttons));
}
