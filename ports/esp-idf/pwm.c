/*
 * SPDX-FileCopyrightText: 2023 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "driver/ledc.h"
#include "libmcu/pwm.h"
#include "libmcu/assert.h"

#if !defined(DEFAULT_ESP_LEDC_TIMER_BIT)
#define DEFAULT_ESP_LEDC_TIMER_BIT	LEDC_TIMER_9_BIT
#endif

struct pwm {
	struct pwm_api api;

	ledc_timer_t timer;
	uint8_t channel;
	ledc_mode_t speed_mode;
	ledc_timer_bit_t duty_resolution;
	uint32_t freq_hz;
	int gpio_num;
};

static void initialize_ledc(struct pwm *self)
{
	ledc_timer_config_t ledc_timer = {
		.speed_mode       = self->speed_mode,
		.timer_num        = self->timer,
		.duty_resolution  = self->duty_resolution,
		.freq_hz          = self->freq_hz,
		.clk_cfg          = LEDC_AUTO_CLK,
	};
	ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

	ledc_channel_config_t ledc_channel = {
		.speed_mode     = self->speed_mode,
		.channel        = self->channel,
		.timer_sel      = self->timer,
		.intr_type      = LEDC_INTR_DISABLE,
		.gpio_num       = self->gpio_num,
		.duty           = 0,
		.hpoint         = 0,
	};
	ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

static int update_frequency(struct pwm *self, int ch, uint32_t hz)
{
	ledc_set_freq(self->speed_mode, self->timer, hz);
	return 0;
}

static int update_duty(struct pwm *self, int ch, uint32_t millipercent)
{
	uint32_t resolution = 1024; /* 10-bit */

	switch (self->duty_resolution) {
	case LEDC_TIMER_13_BIT:
		resolution = 8192;
		break;
	case LEDC_TIMER_10_BIT:
		resolution = 1024;
		break;
	case LEDC_TIMER_9_BIT:
		resolution = 512;
		break;
	case LEDC_TIMER_8_BIT: /* fall through */
	default:
		resolution = 256;
		break;
	}

	uint32_t duty = resolution * millipercent / 100000;
	duty = duty > 0? duty - 1 : duty;

	ledc_set_duty(self->speed_mode, self->channel, duty);
	ledc_update_duty(self->speed_mode, self->channel);
	return 0;
}

static int start_pwm(struct pwm *self,
		uint32_t freq_hz, uint32_t duty_millipercent)
{
	update_frequency(self, 0, freq_hz);
	update_duty(self, 0, duty_millipercent);
	ledc_update_duty(self->speed_mode, self->channel);
	return 0;
}

static int stop_pwm(struct pwm *self)
{
	ledc_stop(self->speed_mode, self->channel, 0);
	return 0;
}

static int enable_pwm(struct pwm *self)
{
	initialize_ledc(self);
	return 0;
}

static int disable_pwm(struct pwm *self)
{
	return 0;
}

struct pwm *pwm_create(uint8_t ch, int pin)
{
	assert(ch < LEDC_TIMER_MAX);

	static struct pwm pwms[LEDC_TIMER_MAX];
	struct pwm *pwm = &pwms[ch];

	pwm->api = (struct pwm_api) {
		.enable = enable_pwm,
		.disable = disable_pwm,
		.start = start_pwm,
		.update_frequency = update_frequency,
		.update_duty = update_duty,
		.stop = stop_pwm,
	};

	pwm->timer = ch;
	pwm->channel = 0;
	pwm->speed_mode = LEDC_LOW_SPEED_MODE;
	pwm->duty_resolution = DEFAULT_ESP_LEDC_TIMER_BIT;
	pwm->freq_hz = 1000;
	pwm->gpio_num = pin;

	return pwm;
}

int pwm_delete(struct pwm *self)
{
	return 0;
}
