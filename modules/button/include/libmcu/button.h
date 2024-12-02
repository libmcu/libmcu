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
} button_error_t;

typedef enum {
	BUTTON_LEVEL_LOW, /**< Released */
	BUTTON_LEVEL_HIGH, /**< Pressed */
} button_level_t;

typedef enum {
	BUTTON_STATE_UNKNOWN,
	BUTTON_STATE_PRESSED,
	BUTTON_STATE_RELEASED,
	BUTTON_STATE_HOLDING,
} button_state_t;

struct button_param {
	uint16_t sampling_interval_ms; /**< interval to check the button state */
	uint16_t min_press_time_ms; /**< minimum time to consider as a press */
	uint16_t repeat_delay_ms; /**< delay before the repeat event */
	uint16_t repeat_rate_ms; /**< rate of the repeat event */
	uint16_t click_window_ms; /**< time window to consider as a click */
	uint16_t max_sampling_interval_ms; /**< maximum interval to check
						the button state */
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
 * @param[in] repeats The number of repeat presses detected.
 * @param[in] ctx Context pointer passed to the callback function.
 */
typedef void (*button_callback_t)(struct button *button,
		const button_state_t event, const uint16_t clicks,
		const uint16_t repeats, void *ctx);

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
button_error_t button_get_param(const struct button *btn,
		struct button_param *param);

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
 * @brief Steps the button state machine by the elapsed time.
 *
 * @param[in] btn Pointer to the button instance.
 * @param[in] time_elapsed_ms Elapsed time in milliseconds to
 *	      step the state machine.
 *
 * @return Error code indicating success or failure.
 */
button_error_t button_step_elapsed(struct button *btn,
		const uint32_t time_elapsed_ms);

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
bool button_busy(const struct button *btn);

/**
 * @brief Gets the current state of the button.
 *
 * @param[in] btn Pointer to the button instance.
 *
 * @return The current state of the button.
 */
button_state_t button_state(const struct button *btn);

/**
 * @brief Gets the number of clicks detected for the button.
 *
 * @param[in] btn Pointer to the button instance.
 *
 * @return The number of clicks detected.
 */
uint16_t button_clicks(const struct button *btn);

/**
 * @brief Gets the number of repeat presses detected for the button.
 *
 * @param[in] btn Pointer to the button instance.
 *
 * @return The number of repeat presses detected.
 */
uint16_t button_repeats(const struct button *btn);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_BUTTON_H */
