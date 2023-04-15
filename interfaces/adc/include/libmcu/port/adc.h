/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_ADC_PORT_H
#define LIBMCU_ADC_PORT_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "libmcu/adc.h"

struct adc *adc_port_create(uint8_t adc_num);
int adc_port_delete(struct adc *self);
int adc_port_enable(struct adc *self);
int adc_port_disable(struct adc *self);
int adc_port_channel_init(struct adc *self, adc_channel_t channel);
int adc_port_calibrate(struct adc *self);
int adc_port_measure(struct adc *self);
int adc_port_read(struct adc *self, adc_channel_t channel);
int adc_port_convert_to_millivolts(struct adc *self, int value);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_ADC_PORT_H */
