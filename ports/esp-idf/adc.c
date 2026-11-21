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
#include "adc_range.h"

#if !defined(ADC_MAX)
#define ADC_MAX			2
#endif
#if !defined(ADC_RESULUTION)
#define ADC_RESOLUTION		ADC_BITWIDTH_12
#endif
#if !defined(ADC_ATTEN)
#define ADC_ATTEN		ADC_ATTEN_DB_12
#endif

#if defined(esp32s3) || defined(esp32c3) || \
	defined(CONFIG_IDF_TARGET_ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32C3)
#define cal_type		adc_cali_curve_fitting_config_t
#define cal_func		adc_cali_create_scheme_curve_fitting
#define cal_del			adc_cali_delete_scheme_curve_fitting
#else
#define cal_type		adc_cali_line_fitting_config_t
#define cal_func		adc_cali_create_scheme_line_fitting
#define cal_del			adc_cali_delete_scheme_line_fitting
#endif

struct lm_adc {
	struct lm_adc_api api;

	adc_oneshot_unit_handle_t handle;
	adc_cali_handle_t cal_handles[SOC_ADC_ATTEN_NUM];
	uint8_t channel_range[SOC_ADC_MAX_CHANNEL_NUM];
	uint8_t unit;
	bool activated;
};

static const adc_atten_t range_atten[] = {
	[LM_ADC_RANGE_DEFAULT] = ADC_ATTEN,
	[LM_ADC_RANGE_0_DB] = ADC_ATTEN_DB_0,
	[LM_ADC_RANGE_2_5_DB] = ADC_ATTEN_DB_2_5,
	[LM_ADC_RANGE_6_DB] = ADC_ATTEN_DB_6,
	[LM_ADC_RANGE_12_DB] = ADC_ATTEN_DB_12,
};

static int channel_to_esp(lm_adc_channel_t channel)
{
	return channel? __builtin_ctz(channel) + 1 : 0;
}

static int calibrate_atten(struct lm_adc *self, adc_atten_t atten)
{
	if (self->cal_handles[atten]) {
		return 0;
	}

	cal_type cfg = {
		.unit_id = self->unit,
		.atten = atten,
		.bitwidth = ADC_RESOLUTION,
	};

	if (cal_func(&cfg, &self->cal_handles[atten]) != ESP_OK) {
		return -ESTALE;
	}

	return 0;
}

static int raw_to_millivolts(adc_cali_handle_t handle, int value)
{
	int mv;

	if (!handle || adc_cali_raw_to_voltage(handle, value, &mv) != ESP_OK) {
		return -ENOSR;
	}

	return mv;
}

static int convert_to_millivolts(struct lm_adc *self, int value)
{
	if (!self || !self->activated) {
		return -EPIPE;
	}

	return raw_to_millivolts(self->cal_handles[ADC_ATTEN], value);
}

static int convert_to_millivolts_channel(struct lm_adc *self,
		lm_adc_channel_t channel, int value)
{
	int ch = channel_to_esp(channel);

	if (!self || !self->activated) {
		return -EPIPE;
	}
	if (!channel || ch >= SOC_ADC_CHANNEL_NUM(self->unit)) {
		return -EINVAL;
	}

	int range = self->channel_range[ch];
	if (range == LM_ADC_RANGE_DEFAULT) {
		return convert_to_millivolts(self, value);
	}

	adc_atten_t atten = range_atten[range];
	int err = calibrate_atten(self, atten);
	return err? err : raw_to_millivolts(self->cal_handles[atten], value);
}

static int read_adc(struct lm_adc *self, lm_adc_channel_t channel)
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

static int measure(struct lm_adc *self)
{
	if (!self || !self->activated) {
		return -EPIPE;
	}

	return 0;
}

static int configure_channel(struct lm_adc *self, lm_adc_channel_t channel,
		enum lm_adc_range range)
{
	if (!self || !self->activated) {
		return -EPIPE;
	}

	int ch = channel_to_esp(channel);
	if (!channel || ch >= SOC_ADC_CHANNEL_NUM(self->unit)) {
		return -EINVAL;
	}

	adc_oneshot_chan_cfg_t cfg = {
		.bitwidth = ADC_RESOLUTION,
		.atten = range_atten[range],
	};

	if (adc_oneshot_config_channel(self->handle, ch, &cfg) != ESP_OK) {
		return -EINVAL;
	}

	self->channel_range[ch] = (uint8_t)range;
	return 0;
}

static int init_channel(struct lm_adc *self, lm_adc_channel_t channel)
{
	return configure_channel(self, channel, LM_ADC_RANGE_DEFAULT);
}

static int init_channel_param(struct lm_adc *self, lm_adc_channel_t channel,
		const struct lm_adc_channel_param *param)
{
	if (!param || !channel) {
		return -EINVAL;
	}

	int range = lm_adc_range_from_mv(param->max_input_mv);
	if (range < 0) {
		return range;
	}

	return configure_channel(self, channel, (enum lm_adc_range)range);
}

static int calibrate_adc(struct lm_adc *self)
{
	if (!self || !self->activated) {
		return -EPIPE;
	}

	return calibrate_atten(self, ADC_ATTEN);
}

static int enable_adc(struct lm_adc *self)
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

static int disable_adc(struct lm_adc *self)
{
	if (!self) {
		return -EPIPE;
	} else if (!self->activated) {
		return -EALREADY;
	}

	adc_oneshot_del_unit(self->handle);
	for (int i = 0; i < SOC_ADC_ATTEN_NUM; i++) {
		if (self->cal_handles[i]) {
			cal_del(self->cal_handles[i]);
			self->cal_handles[i] = NULL;
		}
	}
	memset(self->channel_range, 0, sizeof(self->channel_range));

	self->activated = false;
	return 0;
}

struct lm_adc *lm_adc_create(uint8_t adc_num)
{
	static struct lm_adc adc[ADC_MAX];

	if (--adc_num >= ADC_MAX || adc[adc_num].activated) {
		return NULL;
	}

	for (int i = 0; i < ADC_MAX; i++) {
		adc[i].api = (struct lm_adc_api) {
			.enable = enable_adc,
			.disable = disable_adc,
			.init_channel = init_channel,
			.calibrate = calibrate_adc,
			.measure = measure,
			.read = read_adc,
			.convert_to_millivolts = convert_to_millivolts,
			.init_channel_param = init_channel_param,
			.convert_to_millivolts_channel =
				convert_to_millivolts_channel,
		};
	};

	adc[adc_num].unit = adc_num;

	return &adc[adc_num];
}

int lm_adc_delete(struct lm_adc *self)
{
	if (!self) {
		return -EPIPE;
	} else if (self->activated) {
		lm_adc_disable(self);
	}

	memset(self, 0, sizeof(*self));
	return 0;
}
