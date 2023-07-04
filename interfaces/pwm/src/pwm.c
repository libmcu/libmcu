/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/pwm.h"
#include "libmcu/port/pwm.h"

#include <errno.h>

int pwm_start(struct pwm *pwm, uint32_t freq_hz, uint32_t duty_millipercent)
{
	if (!pwm->enabled) {
		return -ENODEV;
	}
	if (pwm->running) {
		return -EALREADY;
	}

	int err = pwm_port_start(pwm, freq_hz, duty_millipercent);

	if (!err) {
		pwm->running = true;
	}

	return err;
}

int pwm_update_frequency(struct pwm *pwm, uint32_t hz)
{
	if (!pwm->enabled) {
		return -ENODEV;
	}

	return pwm_port_update_frequency(pwm, hz);
}

int pwm_update_duty(struct pwm *pwm, uint32_t millipercent)
{
	if (!pwm->enabled) {
		return -ENODEV;
	}

	return pwm_port_update_duty(pwm, millipercent);
}

int pwm_stop(struct pwm *pwm)
{
	if (!pwm->enabled) {
		return -ENODEV;
	}
	if (!pwm->running) {
		return -EALREADY;
	}

	pwm->running = false;
	return pwm_port_stop(pwm);
}

int pwm_enable(struct pwm *pwm)
{
	if (pwm->enabled) {
		return -EALREADY;
	}

	int err = pwm_port_enable(pwm);

	if (!err) {
		pwm->enabled = true;
	}

	return err;
}

int pwm_disable(struct pwm *pwm)
{
	if (!pwm->enabled) {
		return -ENODEV;
	}
	if (pwm->running) {
		pwm_stop(pwm);
	}

	pwm->enabled = false;
	return pwm_port_disable(pwm);
}

struct pwm *pwm_create(uint8_t ch)
{
	return pwm_port_create(ch);
}

int pwm_delete(struct pwm *pwm)
{
	pwm_stop(pwm);
	pwm_disable(pwm);
	return pwm_port_delete(pwm);
}
