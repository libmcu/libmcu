/*
 * SPDX-FileCopyrightText: 2023 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_BUZZER_H
#define LIBMCU_BUZZER_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdbool.h>
#include "libmcu/pwm.h"
#include "melody.h"

typedef struct buzzer_callbacks {
	void (*on_frequency_changed)(void *ctx, uint16_t hz);
	void (*on_stop)(void *ctx);
} buzzer_callback_t;

void buzzer_init(struct pwm_channel *pwm,
		buzzer_callback_t *on_event, void *on_event_ctx);
void buzzer_play(const struct melody *melody);
void buzzer_mute(void);
void buzzer_unmute(void);
bool buzzer_busy(void);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_BUZZER_H */
