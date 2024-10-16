/*
 * SPDX-FileCopyrightText: 2023 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/pwm.h"

#include <string.h>
#include <errno.h>

#include "libmcu/assert.h"
#include "driver/ledc.h"

#if !defined(DEFAULT_ESP_LEDC_TIMER_BIT)
#define DEFAULT_ESP_LEDC_TIMER_BIT	LEDC_TIMER_9_BIT
#endif

struct pwm_channel {
	struct pwm *pwm;

	uint8_t id;
	int pin;

	bool enabled;
};

struct pwm {
	ledc_timer_t timer;
	ledc_mode_t speed_mode;
	ledc_timer_bit_t duty_resolution;
	uint32_t freq_hz;

	struct pwm_channel *channels[LEDC_CHANNEL_MAX];
};

/* NOTE: the timers are multiplexed to the ledc channels. thus the same channel
 * for different timers will not work. */
static struct pwm_channel channels[LEDC_CHANNEL_MAX];

static struct pwm_channel *alloc_channel(struct pwm *self, int ch, int pin)
{
	struct pwm_channel *channel = &channels[ch];

	if (channel->pwm) {
		return NULL;
	}

	channel->pwm = self;
	channel->id = ch;
	channel->pin = pin;

	return channel;
}

static void free_channel(struct pwm_channel *ch)
{
	ch->pwm = NULL;
}

static bool is_timer_enabled(struct pwm *self)
{
	for (int i = 0; i < LEDC_CHANNEL_MAX; i++) {
		if (self->channels[i]) {
			return true;
		}
	}

	return false;
}

static void initialize_ledc(struct pwm *self, int ch, int pin)
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
		.channel        = ch,
		.timer_sel      = self->timer,
		.intr_type      = LEDC_INTR_DISABLE,
		.gpio_num       = pin,
		.duty           = 0,
		.hpoint         = 0,
	};
	ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

static int update_frequency(struct pwm_channel *ch, uint32_t hz)
{
	ledc_set_freq(ch->pwm->speed_mode, ch->pwm->timer, hz);
	return 0;
}

static int update_duty(struct pwm_channel *ch, uint32_t millipercent)
{
	uint32_t resolution = 1024; /* 10-bit */

	switch (ch->pwm->duty_resolution) {
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

	ledc_set_duty(ch->pwm->speed_mode, ch->id, duty);
	ledc_update_duty(ch->pwm->speed_mode, ch->id);

	return 0;
}

int pwm_update_duty(struct pwm_channel *ch, uint32_t millipercent)
{
	return update_duty(ch, millipercent);
}

int pwm_update_frequency(struct pwm_channel *ch, uint32_t hz)
{
	return update_frequency(ch, hz);
}

int pwm_start(struct pwm_channel *ch,
		uint32_t freq_hz, uint32_t duty_millipercent)
{
	update_frequency(ch, freq_hz);
	update_duty(ch, duty_millipercent);
	return 0;
}

int pwm_stop(struct pwm_channel *ch)
{
	ledc_stop(ch->pwm->speed_mode, ch->id, 0);
	return 0;
}

int pwm_enable(struct pwm_channel *ch)
{
	if (ch->enabled) {
		return -EALREADY;
	}

	initialize_ledc(ch->pwm, ch->id, ch->pin);
	ch->enabled = true;

	return 0;
}

int pwm_disable(struct pwm_channel *ch)
{
	ch->enabled = false;
	return 0;
}

struct pwm_channel *pwm_create_channel(struct pwm *self, int ch, int pin)
{
	self->channels[ch] = alloc_channel(self, ch, pin);

	if (self->channels[ch]) {
		/* default */
		self->speed_mode = LEDC_LOW_SPEED_MODE;
		self->duty_resolution = DEFAULT_ESP_LEDC_TIMER_BIT;
		self->freq_hz = 1000;
	}

	return (struct pwm_channel *)self->channels[ch];
}

int pwm_delete_channel(struct pwm_channel *ch)
{
	free_channel(ch);

	if (!is_timer_enabled(ch->pwm)) {
	}

	return 0;
}

struct pwm *pwm_create(uint8_t timer)
{
	assert(timer < LEDC_TIMER_MAX);

	static struct pwm pwms[LEDC_TIMER_MAX];
	struct pwm *pwm = &pwms[timer];

	memset(pwm, 0, sizeof(*pwm));
	pwm->timer = timer;

	return pwm;
}

int pwm_delete(struct pwm *self)
{
	return 0;
}
