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

#define PWM_PCT_TO_MILLI(pct)		((pct) * 1000)
#define PWM_MILLI_TO_PCT(millipct)	((millipct) / 1000)

struct pwm;

struct pwm_api {
	int (*enable)(struct pwm *self);
	int (*disable)(struct pwm *self);
	int (*start)(struct pwm *self,
			uint32_t freq_hz, uint32_t duty_millipercent);
	int (*update_frequency)(struct pwm *self, int ch, uint32_t hz);
	int (*update_duty)(struct pwm *self, int ch, uint32_t millipercent);
	int (*stop)(struct pwm *self);
};

static inline int pwm_enable(struct pwm *self) {
	return ((struct pwm_api *)self)->enable(self);
}

static inline int pwm_disable(struct pwm *self) {
	return ((struct pwm_api *)self)->disable(self);
}

static inline int pwm_start(struct pwm *self,
		uint32_t freq_hz, uint32_t duty_millipercent) {
	return ((struct pwm_api *)self)->start(self,
			freq_hz, duty_millipercent);
}

static inline int pwm_update_frequency(struct pwm *self, int ch, uint32_t hz) {
	return ((struct pwm_api *)self)->update_frequency(self, ch, hz);
}

static inline int pwm_update_duty(struct pwm *self,
		int ch, uint32_t millipercent) {
	return ((struct pwm_api *)self)->update_duty(self, ch, millipercent);
}

static inline int pwm_stop(struct pwm *self) {
	return ((struct pwm_api *)self)->stop(self);
}

struct pwm *pwm_create(uint8_t ch);
int pwm_delete(struct pwm *self);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_PWM_H */
