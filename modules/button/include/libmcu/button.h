/*
 * SPDX-FileCopyrightText: 2021 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_BUTTON_H
#define LIBMCU_BUTTON_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>

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
#if !defined(BUTTON_REPEAT_RATE_MS)
#define BUTTON_REPEAT_RATE_MS			200U
#endif
#if !defined(BUTTON_CLICK_WINDOW_MS)
#define BUTTON_CLICK_WINDOW_MS			500U
#endif

typedef enum {
	BUTTON_BUSY, /**< Too many calls in a sampling period */
	BUTTON_SCANNING, /**< Activity detected on buttons. Scanning for state */
	BUTTON_NO_ACTIVITY, /**< No activity detected on buttons */
} button_rc_t;

enum button_event {
	BUTTON_EVT_PRESSED,
	BUTTON_EVT_RELEASED,
	BUTTON_EVT_HOLDING,
	BUTTON_EVT_CLICK,
};

struct button_data {
	unsigned int history;
	unsigned long time_pressed;
	unsigned long time_released;
	unsigned long time_repeat;
	uint8_t click; /**< the number of clicks */
};

typedef void (*button_handler_t)(enum button_event event,
		const struct button_data *info, void *ctx);

void button_init(unsigned long (*get_time_ms)(void));

/**
 * Register a button
 *
 * @param[in] get_button_state a function to get the button state
 * @param[in] handlers @ref struct button_handlers
 * @param[in] ctx user context
 *
 * @return a handle if registered successfully. NULL otherwise
 */
const void *button_register(int (*get_button_state)(void),
		button_handler_t handler, void *ctx);

/**
 * Replace the button handler
 *
 * @param[in] btn a button handle which is given by @ref button_register
 * @param[in] handler @ref button_handler_t
 * @param[in] ctx user context
 *
 * @return a handle if registered successfully. NULL otherwise
 */
void button_update_handler(void *btn, button_handler_t handler, void *ctx);

/**
 * Scan all buttons and update the states
 *
 * @param[in] context user context
 *
 * @return @ref button_rc_t
 *
 * @note On 32-bit systems the millisecond time cycles approximately every 50
 * days. In case of the wraparound, clicks may resulted in false-positive if
 * not button_step() called periodic until BUTTON_NO_ACTIVITY returned.
 *
 * @warn The millisecond time wraparound may add latency by @ref
 * BUTTON_SAMPLIING_PERIOD_MS.
 */
button_rc_t button_step(void);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_BUTTON_H */
