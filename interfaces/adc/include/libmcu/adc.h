/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_ADC_H
#define LIBMCU_ADC_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>

#define adc_enable		libmcu_adc_enable
#define adc_disable		libmcu_adc_disable
#define adc_channel_init	libmcu_adc_channel_init
#define adc_calibrate		libmcu_adc_calibrate

struct adc;

struct adc *adc_create(uint8_t adc_num);
int adc_delete(struct adc *self);
int adc_enable(struct adc *self);
int adc_disable(struct adc *self);
int adc_channel_init(struct adc *self, uint16_t channel);
int adc_calibrate(struct adc *self);
int adc_measure(struct adc *self);
int adc_convert_to_millivolts(struct adc *self, int value);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_ADC_H */
