/*
 * SPDX-FileCopyrightText: 2023 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_BUZZER_H
#define LIBMCU_BUZZER_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdbool.h>
#include "melody.h"

typedef struct buzzer_callbacks {
	void (*on_frequency_changed)(void *ctx, uint16_t hz);
	void (*on_stop)(void *ctx);
} buzzer_callback_t;

struct lm_pwm_channel;

/**
 * @brief Initialize the buzzer module.
 *
 * @param[in] pwm Pointer to the PWM channel structure.
 * @param[in] on_event Callback function to be called on buzzer events.
 * @param[in] on_event_ctx Context pointer to be passed to the callback
 *            function.
 */
void buzzer_init(struct lm_pwm_channel *pwm,
		buzzer_callback_t *on_event, void *on_event_ctx);

/**
 * @brief Play a melody using the buzzer.
 *
 * @param[in] melody Pointer to the melody structure to be played.
 */
void buzzer_play(const struct melody *melody);

/**
 * @brief Mute the buzzer.
 */
void buzzer_mute(void);

/**
 * @brief Unmute the buzzer.
 */
void buzzer_unmute(void);

/**
 * @brief Check if the buzzer is currently busy playing a melody.
 *
 * @return true if the buzzer is busy, false otherwise.
 */
bool buzzer_busy(void);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_BUZZER_H */
