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
struct pwm_channel;

/**
 * @brief Create a PWM instance.
 *
 * This function creates a PWM instance associated with the given timer.
 *
 * @param[in] timer The timer to be associated with the PWM instance.
 *
 * @return A pointer to the created PWM instance. If the creation fails,
 *         the function returns NULL.
 */
struct pwm *pwm_create(uint8_t timer);

/**
 * @brief Delete a PWM instance.
 *
 * This function deletes a PWM instance and frees the associated resources.
 *
 * @param[in] self The PWM instance to be deleted.
 *
 * @return 0 if the deletion is successful, otherwise returns a non-zero error code.
 */
int pwm_delete(struct pwm *self);

/**
 * Creates a new PWM channel.
 *
 * @param[in] self A pointer to the PWM structure.
 * @param[in] ch The channel number.
 * @param[in] pin The pin number.
 *
 * @return A pointer to the created PWM channel.
 */
struct pwm_channel *pwm_create_channel(struct pwm *self, int ch, int pin);

/**
 * Deletes a PWM channel.
 *
 * @param[in] ch A pointer to the PWM channel to delete.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int pwm_delete_channel(struct pwm_channel *ch);

/**
 * Enables a PWM channel.
 *
 * @param[in] ch A pointer to the PWM channel to enable.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int pwm_enable(struct pwm_channel *ch);

/**
 * Disables a PWM channel.
 *
 * @param[in] ch A pointer to the PWM channel to disable.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int pwm_disable(struct pwm_channel *ch);

/**
 * @brief Start a PWM channel.
 *
 * This function starts a PWM channel with a given frequency and duty cycle.
 *
 * @param[in] ch The PWM channel to be started.
 * @param[in] freq_hz The frequency of the PWM signal in Hz.
 * @param[in] duty_millipercent The duty cycle of the PWM signal in millipercent.
 *
 * @return 0 if the operation is successful, otherwise returns a non-zero error code.
 */
int pwm_start(struct pwm_channel *ch, uint32_t freq_hz, uint32_t duty_millipercent);

/**
 * @brief Stop a PWM channel.
 *
 * This function stops a PWM channel and resets its configuration.
 *
 * @param[in] ch The PWM channel to be stopped.
 *
 * @return 0 if the operation is successful, otherwise returns a non-zero error code.
 */
int pwm_stop(struct pwm_channel *ch);

/**
 * @brief Update the frequency of a PWM channel.
 *
 * This function updates the frequency of a PWM channel without stopping it.
 *
 * @param[in] ch The PWM channel whose frequency is to be updated.
 * @param[in] hz The new frequency in Hz.
 *
 * @return 0 if the operation is successful, otherwise returns a non-zero error code.
 */
int pwm_update_frequency(struct pwm_channel *ch, uint32_t hz);

/**
 * @brief Update the duty cycle of a PWM channel.
 *
 * This function updates the duty cycle of a PWM channel without stopping it.
 *
 * @param[in] ch The PWM channel whose duty cycle is to be updated.
 * @param[in] millipercent The new duty cycle in millipercent.
 *
 * @return 0 if the operation is successful, otherwise returns a non-zero error code.
 */
int pwm_update_duty(struct pwm_channel *ch, uint32_t millipercent);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_PWM_H */
