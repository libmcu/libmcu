/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_PWM_H
#define LIBMCU_PWM_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

struct pwm {
	bool enabled;
	bool running;
};

struct pwm *pwm_create(uint8_t ch);
int pwm_delete(struct pwm *pwm);
int pwm_enable(struct pwm *pwm);
int pwm_disable(struct pwm *pwm);
int pwm_start(struct pwm *pwm, uint32_t freq_hz, uint32_t duty_millipercent);
int pwm_update_frequency(struct pwm *pwm, uint32_t hz);
int pwm_update_duty(struct pwm *pwm, uint32_t millipercent);
int pwm_stop(struct pwm *pwm);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_PWM_H */
