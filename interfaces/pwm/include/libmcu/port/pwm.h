/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_PWM_PORT_H
#define LIBMCU_PWM_PORT_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "libmcu/pwm.h"

struct pwm *pwm_port_create(uint8_t ch);
int pwm_port_delete(struct pwm *pwm);
int pwm_port_enable(struct pwm *pwm);
int pwm_port_disable(struct pwm *pwm);
int pwm_port_start(struct pwm *pwm,
		uint32_t freq_hz, uint32_t duty_millipercent);
int pwm_port_update_frequency(struct pwm *pwm, uint32_t hz);
int pwm_port_update_duty(struct pwm *pwm, uint32_t millipercent);
int pwm_port_stop(struct pwm *pwm);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_PWM_PORT_H */
