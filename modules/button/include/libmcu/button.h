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

#include <stdbool.h>

struct button_data {
	unsigned int history;
	unsigned long time_pressed;
	unsigned long time_released;
};

struct button_handlers {
	void (*pressed)(const struct button_data *btn, void *context);
	void (*released)(const struct button_data *btn, void *context);
	void (*holding)(const struct button_data *btn, void *context);
	void (*clicked)(const struct button_data *btn, void *context);
};

void button_init(unsigned long (*get_time_ms)(void));

/**
 * Register a button
 *
 * @param[in] handlers @ref struct button_handlers
 * @param[in] get_button_state a function to get the button state
 *
 * @return a handle if registered successfully. NULL otherwise
 */
const void *button_register(const struct button_handlers *handlers,
		int (*get_button_state)(void));

bool button_is_pressed(const void *handle);

/**
 * Scan all buttons and update the states
 *
 * @param[in] context user context
 *
 * @return false when waiting for the next period. true otherwise
 */
bool button_poll(void *context);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_BUTTON_H */
