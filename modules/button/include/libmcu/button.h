/*
 * SPDX-FileCopyrightText: 2021 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_BUTTON_H
#define LIBMCU_BUTTON_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef enum {
	BUTTON_ERROR_UNKNOWN,
	BUTTON_ERROR_NONE,
	BUTTON_ERROR_INVALID_PARAM,
	BUTTON_ERROR_INCORRECT_PARAM,
	BUTTON_ERROR_DISABLED,
	BUTTON_ERROR_BUSY,
} button_error_t;

typedef enum {
	BUTTON_LEVEL_LOW, /**< Released */
	BUTTON_LEVEL_HIGH, /**< Pressed */
} button_level_t;

typedef enum button_event {
	BUTTON_EVT_NONE,
	BUTTON_EVT_PRESSED,
	BUTTON_EVT_RELEASED,
	BUTTON_EVT_HOLDING,
	BUTTON_EVT_CLICK,
} button_event_t;

struct button_param {
	uint16_t sampling_interval_ms; /**< interval to check the button state */
	uint16_t min_press_time_ms; /**< minimum time to consider as a press */
	uint16_t repeat_delay_ms; /**< delay before the repeat event */
	uint16_t repeat_rate_ms; /**< rate of the repeat event */
	uint16_t click_window_ms; /**< time window to consider as a click */
	uint16_t padding;
};

struct button;

/**
 * @typedef button_get_state_func_t
 * @brief Function pointer type for getting the state of a button.
 *
 * @param[in] ctx Context pointer passed to the function.
 *
 * @return The current state of the button.
 */
typedef button_level_t (*button_get_state_func_t)(void *ctx);

/**
 * @typedef button_callback_t
 * @brief Function pointer type for button event callback.
 *
 * @param[in] button Pointer to the button structure.
 * @param[in] event The event that occurred.
 * @param[in] clicks The number of clicks detected.
 * @param[in] ctx Context pointer passed to the callback function.
 */
typedef void (*button_callback_t)(struct button *button,
		const button_event_t event, const uint8_t clicks, void *ctx);

/**
 * @brief Creates a new button instance.
 *
 * @param[in] f_get Function to get the state of the button.
 * @param[in] f_get_ctx Context for the state function.
 * @param[in] cb Callback function for button events.
 * @param[in] cb_ctx Context for the callback function.
 *
 * @return Pointer to the new button instance.
 */
struct button *button_new(button_get_state_func_t f_get, void *f_get_ctx,
		button_callback_t cb, void *cb_ctx);

/**
 * @brief Deletes a button instance.
 *
 * @param[in] btn Pointer to the button instance to delete.
 */
void button_delete(struct button *btn);

/**
 * @brief Enables a button.
 *
 * @param[in] btn Pointer to the button instance to enable.
 *
 * @return Error code indicating success or failure.
 */
button_error_t button_enable(struct button *btn);

/**
 * @brief Disables a button.
 *
 * @param[in] btn Pointer to the button instance to disable.
 *
 * @return Error code indicating success or failure.
 */
button_error_t button_disable(struct button *btn);

/**
 * @brief Sets parameters for a button.
 *
 * @param[in] btn Pointer to the button instance.
 * @param[in] param Pointer to the parameters to set.
 *
 * @return Error code indicating success or failure.
 */
button_error_t button_set_param(struct button *btn,
		const struct button_param *param);

/**
 * @brief Gets parameters of a button.
 *
 * @param[in] btn Pointer to the button instance.
 * @param[in] param Pointer to store the retrieved parameters.
 *
 * @return Error code indicating success or failure.
 */
button_error_t button_get_param(struct button *btn, struct button_param *param);

/**
 * @brief Steps the button state machine by a given time.
 *
 * @param[in] btn Pointer to the button instance.
 * @param[in] time_ms Time in milliseconds to step the state machine.
 *
 * @return Error code indicating success or failure.
 */
button_error_t button_step(struct button *btn, const uint32_t time_ms);

/**
 * @brief Checks if the button is currently busy.
 *
 * @param btn Pointer to the button instance.
 *
 * @note A button is considered busy when it is in the process of detecting like
 *       in the period of debouncing.
 *
 * @return True if the button is busy, false otherwise.
 */
bool button_busy(struct button *btn);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_BUTTON_H */
