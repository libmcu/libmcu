/*
 * SPDX-FileCopyrightText: 2023 권경환 Kyunghwan Kwon <k@libmcu.org>
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

struct lm_pwm_channel {
	struct lm_pwm *pwm;

	uint8_t id;
	int pin;

	bool enabled;
};

struct lm_pwm {
	ledc_timer_t timer;
	ledc_mode_t speed_mode;
	ledc_timer_bit_t duty_resolution;
	uint32_t freq_hz;

	struct lm_pwm_channel *channels[LEDC_CHANNEL_MAX];
};

/* NOTE: the timers are multiplexed to the ledc channels. thus the same channel
 * for different timers will not work. */
static struct lm_pwm_channel channels[LEDC_CHANNEL_MAX];

static struct lm_pwm_channel *alloc_channel(struct lm_pwm *self,
		int ch, int pin)
{
	struct lm_pwm_channel *channel = &channels[ch];

	if (channel->pwm) {
		return NULL;
	}

	channel->pwm = self;
	channel->id = ch;
	channel->pin = pin;

	return channel;
}

static void free_channel(struct lm_pwm_channel *ch)
{
	ch->pwm = NULL;
}

static bool is_timer_enabled(struct lm_pwm *self)
{
	for (int i = 0; i < LEDC_CHANNEL_MAX; i++) {
		if (self->channels[i]) {
			return true;
		}
	}

	return false;
}

static void initialize_ledc(struct lm_pwm *self, int ch, int pin)
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

static int update_frequency(struct lm_pwm_channel *ch, uint32_t hz)
{
	int err = ledc_set_freq(ch->pwm->speed_mode, ch->pwm->timer, hz);
	return err == ESP_OK ? 0 : -err;
}

static int update_duty(struct lm_pwm_channel *ch, uint32_t millipercent)
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

	int err = ledc_set_duty(ch->pwm->speed_mode, ch->id, duty);
	err |= ledc_update_duty(ch->pwm->speed_mode, ch->id);

	return err == ESP_OK ? 0 : -err;
}

int lm_pwm_update_duty(struct lm_pwm_channel *ch, uint32_t millipercent)
{
	return update_duty(ch, millipercent);
}

int lm_pwm_update_frequency(struct lm_pwm_channel *ch, uint32_t hz)
{
	return update_frequency(ch, hz);
}

int lm_pwm_start(struct lm_pwm_channel *ch,
		uint32_t freq_hz, uint32_t duty_millipercent)
{
	int err = update_frequency(ch, freq_hz);
	err |= update_duty(ch, duty_millipercent);
	return err;
}

int lm_pwm_stop(struct lm_pwm_channel *ch)
{
	int err = ledc_stop(ch->pwm->speed_mode, ch->id, 0);
	return err == ESP_OK ? 0 : -err;
}

int lm_pwm_enable(struct lm_pwm_channel *ch)
{
	if (ch->enabled) {
		return -EALREADY;
	}

	initialize_ledc(ch->pwm, ch->id, ch->pin);
	ch->enabled = true;

	return 0;
}

int lm_pwm_disable(struct lm_pwm_channel *ch)
{
	ch->enabled = false;
	return 0;
}

struct lm_pwm_channel *lm_pwm_create_channel(struct lm_pwm *self,
		int ch, int pin)
{
	self->channels[ch] = alloc_channel(self, ch, pin);

	if (self->channels[ch]) {
		/* default */
		self->speed_mode = LEDC_LOW_SPEED_MODE;
		self->duty_resolution = DEFAULT_ESP_LEDC_TIMER_BIT;
		self->freq_hz = 1000;
	}

	return (struct lm_pwm_channel *)self->channels[ch];
}

int lm_pwm_delete_channel(struct lm_pwm_channel *ch)
{
	free_channel(ch);

	if (!is_timer_enabled(ch->pwm)) {
	}

	return 0;
}

struct lm_pwm *lm_pwm_create(uint8_t timer)
{
	assert(timer < LEDC_TIMER_MAX);

	static struct lm_pwm pwms[LEDC_TIMER_MAX];
	struct lm_pwm *pwm = &pwms[timer];

	memset(pwm, 0, sizeof(*pwm));
	pwm->timer = timer;

	return pwm;
}

int lm_pwm_delete(struct lm_pwm *self)
{
	return 0;
}
