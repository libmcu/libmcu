/*
 * SPDX-FileCopyrightText: 2023 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include <errno.h>
#include <string.h>

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#include "libmcu/adc.h"

#if !defined(ADC_MAX)
#define ADC_MAX			2
#endif
#if !defined(ADC_RESULUTION)
#define ADC_RESOLUTION		ADC_BITWIDTH_12
#endif
#if !defined(ADC_ATTEN)
#define ADC_ATTEN		ADC_ATTEN_DB_12
#endif

#if defined(esp32s3) || defined(esp32c3)
#define cal_type		adc_cali_curve_fitting_config_t
#define cal_func		adc_cali_create_scheme_curve_fitting
#define cal_del			adc_cali_delete_scheme_curve_fitting
#else
#define cal_type		adc_cali_line_fitting_config_t
#define cal_func		adc_cali_create_scheme_line_fitting
#define cal_del			adc_cali_delete_scheme_line_fitting
#endif

struct adc {
	struct adc_api api;

	adc_oneshot_unit_handle_t handle;
	adc_cali_handle_t cal_handle;
	uint8_t unit;
	bool activated;
};

static int channel_to_esp(adc_channel_t channel)
{
	return channel? __builtin_ctz(channel) + 1 : 0;
}

static int convert_to_millivolts(struct adc *self, int value)
{
	int mv;

	if (!self || !self->activated) {
		return -EPIPE;
	}

	if (adc_cali_raw_to_voltage(self->cal_handle, value, &mv) != ESP_OK) {
		return -ENOSR;
	}

	return mv;
}

static int read_adc(struct adc *self, adc_channel_t channel)
{
	int adc;

	if (!self || !self->activated) {
		return -EPIPE;
	}

	if (adc_oneshot_read(self->handle, channel_to_esp(channel), &adc)
			!= ESP_OK) {
		return -ENOMSG;
	}

	return adc;
}

static int measure(struct adc *self)
{
	if (!self || !self->activated) {
		return -EPIPE;
	}

	return 0;
}

static int init_channel(struct adc *self, adc_channel_t channel)
{
	if (!self || !self->activated) {
		return -EPIPE;
	}

	/* 100mV ~ 950mV  : No attenuation
	 * 100mV ~ 1250mV : 2.5dB
	 * 150mV ~ 1750mV : 6dB
	 * 150mV ~ 2450mV : 11dB */
	adc_oneshot_chan_cfg_t cfg = {
		.bitwidth = ADC_RESOLUTION,
		.atten = ADC_ATTEN,
	};

	if (adc_oneshot_config_channel(self->handle,
			channel_to_esp(channel), &cfg) != ESP_OK) {
		return -EINVAL;
	}

	return 0;
}

static int calibrate_adc(struct adc *self)
{
	if (!self || !self->activated) {
		return -EPIPE;
	}

	cal_type cfg = {
		.unit_id = self->unit,
		.atten = ADC_ATTEN,
		.bitwidth = ADC_RESOLUTION,
	};

	if (cal_func(&cfg, &self->cal_handle) != ESP_OK) {
		return -ESTALE;
	}

	return 0;
}

static int enable_adc(struct adc *self)
{
	if (!self) {
		return -EPIPE;
	} else if (self->activated) {
		return -EALREADY;
	}

	adc_oneshot_unit_init_cfg_t cfg = {
		.unit_id = self->unit,
	};

	if (adc_oneshot_new_unit(&cfg, &self->handle) != ESP_OK) {
		return -EFAULT;
	}

	self->activated = true;
	return 0;
}

static int disable_adc(struct adc *self)
{
	if (!self) {
		return -EPIPE;
	} else if (!self->activated) {
		return -EALREADY;
	}

	adc_oneshot_del_unit(self->handle);
	cal_del(self->cal_handle);

	self->activated = false;
	return 0;
}

struct adc *adc_create(uint8_t adc_num)
{
	static struct adc adc[ADC_MAX];

	if (--adc_num >= ADC_MAX || adc[adc_num].activated) {
		return NULL;
	}

	for (int i = 0; i < ADC_MAX; i++) {
		adc[i].api = (struct adc_api) {
			.enable = enable_adc,
			.disable = disable_adc,
			.init_channel = init_channel,
			.calibrate = calibrate_adc,
			.measure = measure,
			.read = read_adc,
			.convert_to_millivolts = convert_to_millivolts,
		};
	};

	adc[adc_num].unit = adc_num;

	return &adc[adc_num];
}

int adc_delete(struct adc *self)
{
	if (!self) {
		return -EPIPE;
	} else if (self->activated) {
		adc_disable(self);
	}

	memset(self, 0, sizeof(*self));
	return 0;
}
