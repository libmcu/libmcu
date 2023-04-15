/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/adc.h"
#include "libmcu/port/adc.h"

struct adc *adc_create(uint8_t adc_num)
{
	return adc_port_create(adc_num);
}

int adc_delete(struct adc *self)
{
	return adc_port_delete(self);
}

int adc_enable(struct adc *self)
{
	return adc_port_enable(self);
}

int adc_disable(struct adc *self)
{
	return adc_port_disable(self);
}

int adc_channel_init(struct adc *self, uint16_t channel)
{
	return adc_port_channel_init(self, channel);
}

int adc_calibrate(struct adc *self)
{
	return adc_port_calibrate(self);
}

int adc_measure(struct adc *self)
{
	return adc_port_measure(self);
}

int adc_convert_to_millivolts(struct adc *self, int value)
{
	return adc_port_convert_to_millivolts(self, value);
}
