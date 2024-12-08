/*
 * SPDX-FileCopyrightText: 2023 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/buzzer.h"

#include <stddef.h>
#include <stdint.h>
#include <semaphore.h>

#include "libmcu/timer.h"

struct playing {
	const struct melody *melody;
	uint8_t index;
};

static struct buzzer {
	struct playing playing;

	buzzer_callback_t *callback;
	void *callback_ctx;

	struct pwm_channel *pwm;
	struct apptmr *timer;

	sem_t lock;
	bool enabled;
} m;

static void on_timeout(struct apptmr *timer, void *arg)
{
	struct buzzer *buzzer = (struct buzzer *)arg;
	struct playing *playing = &buzzer->playing;
	struct pwm_channel *pwm = buzzer->pwm;

	const struct tone *prev = &playing->melody->tones[playing->index];
	playing->index += 1;

	if (playing->index >= playing->melody->nr_tones) {
		pwm_stop(pwm);

		if (buzzer->callback && buzzer->callback->on_stop) {
			buzzer->callback->on_stop(buzzer->callback_ctx);
		}

		return;
	}

	const struct tone *tone = &playing->melody->tones[playing->index];
	tone_pitch_t pitch = tone_get(tone->note, tone->octave);

	if (!pitch) {
		pwm_update_duty(pwm, 0);
	} else {
		if (tone_get(prev->note, prev->octave) == 0) {
			pwm_update_duty(pwm, PWM_PCT_TO_MILLI(50));
		}
		pwm_update_frequency(pwm, pitch);
	}

	if (buzzer->callback && buzzer->callback->on_frequency_changed) {
		buzzer->callback->on_frequency_changed(buzzer->callback_ctx,
				pitch);
	}

	apptmr_start(timer, tone->len_ms);
}

static void play(struct buzzer *buzzer, const struct melody *melody)
{
	apptmr_stop(buzzer->timer);
	pwm_stop(buzzer->pwm);

	buzzer->playing.melody = melody;
	buzzer->playing.index = 0;

	const struct tone *tone = &buzzer->playing.melody->tones[0];
	tone_pitch_t pitch = tone_get(tone->note, tone->octave);
	pwm_start(buzzer->pwm, pitch, PWM_PCT_TO_MILLI(50));

	apptmr_start(buzzer->timer, tone->len_ms);
}

void buzzer_play(const struct melody *melody)
{
	sem_wait(&m.lock);
	if (m.enabled) {
		play(&m, melody);
	}
	sem_post(&m.lock);
}

void buzzer_mute(void)
{
	sem_wait(&m.lock);
	pwm_stop(m.pwm);
	m.enabled = false;
	sem_post(&m.lock);
}

void buzzer_unmute(void)
{
	sem_wait(&m.lock);
	m.enabled = true;
	sem_post(&m.lock);
}

bool buzzer_busy(void)
{
	if (m.enabled && m.playing.melody &&
			m.playing.index < m.playing.melody->nr_tones) {
		return true;
	}

	return false;
}

void buzzer_init(struct pwm_channel *pwm,
		buzzer_callback_t *on_event, void *on_event_ctx)
{
	sem_init(&m.lock, 0, 1);

	m.callback = on_event;
	m.callback_ctx = on_event_ctx;

	m.pwm = pwm;
	m.timer = apptmr_create(false, on_timeout, &m);

	pwm_enable(m.pwm);
	apptmr_enable(m.timer);
	m.enabled = true;
}
