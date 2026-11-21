/*
 * SPDX-FileCopyrightText: 2022 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_ADC_H
#define LIBMCU_ADC_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <errno.h>
#include <stdint.h>

typedef enum {
	LM_ADC_CH_1		= 0x00000001UL,
	LM_ADC_CH_2		= 0x00000002UL,
	LM_ADC_CH_3		= 0x00000004UL,
	LM_ADC_CH_4		= 0x00000008UL,
	LM_ADC_CH_5		= 0x00000010UL,
	LM_ADC_CH_6		= 0x00000020UL,
	LM_ADC_CH_7		= 0x00000040UL,
	LM_ADC_CH_8		= 0x00000080UL,
	LM_ADC_CH_9		= 0x00000100UL,
	LM_ADC_CH_10		= 0x00000200UL,
	LM_ADC_CH_11		= 0x00000400UL,
	LM_ADC_CH_12		= 0x00000800UL,
	LM_ADC_CH_13		= 0x00001000UL,
	LM_ADC_CH_14		= 0x00002000UL,
	LM_ADC_CH_15		= 0x00004000UL,
	LM_ADC_CH_16		= 0x00008000UL,
	LM_ADC_CH_17		= 0x00010000UL,
	LM_ADC_CH_18		= 0x00020000UL,
	LM_ADC_CH_19		= 0x00040000UL,
	LM_ADC_CH_20		= 0x00080000UL,
	LM_ADC_CH_ALL		= 0x7fffffffUL,
} lm_adc_channel_t;

struct lm_adc;

struct lm_adc_channel_param {
	uint32_t max_input_mv; /**< Highest input voltage to measure, in mV. */
};

struct lm_adc_api {
	int (*enable)(struct lm_adc *self);
	int (*disable)(struct lm_adc *self);
	int (*init_channel)(struct lm_adc *self, lm_adc_channel_t channel);
	int (*calibrate)(struct lm_adc *self);
	int (*measure)(struct lm_adc *self);
	int (*read)(struct lm_adc *self, lm_adc_channel_t channel);
	int (*convert_to_millivolts)(struct lm_adc *self, int value);
	int (*init_channel_param)(struct lm_adc *self, lm_adc_channel_t channel,
			const struct lm_adc_channel_param *param);
	int (*convert_to_millivolts_channel)(struct lm_adc *self,
			lm_adc_channel_t channel, int value);
};

static inline int lm_adc_enable(struct lm_adc *self) {
	return ((struct lm_adc_api *)self)->enable(self);
}

static inline int lm_adc_disable(struct lm_adc *self) {
	return ((struct lm_adc_api *)self)->disable(self);
}

static inline int lm_adc_channel_init(struct lm_adc *self,
		lm_adc_channel_t channel) {
	return ((struct lm_adc_api *)self)->init_channel(self, channel);
}

/**
 * @brief Configure a channel for the requested input voltage range.
 *
 * @param[in] self The ADC instance.
 * @param[in] channel The channel to configure.
 * @param[in] param The channel parameters.
 *
 * @return Zero on success or a negative error code on failure.
 * @retval -EINVAL The channel is invalid or param is NULL.
 * @retval -ERANGE The requested range is too large.
 * @retval -ENOTSUP The port omits this operation.
 */
static inline int lm_adc_channel_init_param(struct lm_adc *self,
		lm_adc_channel_t channel,
		const struct lm_adc_channel_param *param) {
	const struct lm_adc_api *api = (const struct lm_adc_api *)self;

	if (!api->init_channel_param) {
		return -ENOTSUP;
	}
	return api->init_channel_param(self, channel, param);
}

static inline int lm_adc_calibrate(struct lm_adc *self) {
	return ((struct lm_adc_api *)self)->calibrate(self);
}

static inline int lm_adc_measure(struct lm_adc *self) {
	return ((struct lm_adc_api *)self)->measure(self);
}

static inline int lm_adc_read(struct lm_adc *self, lm_adc_channel_t channel) {
	return ((struct lm_adc_api *)self)->read(self, channel);
}

static inline int lm_adc_convert_to_millivolts(struct lm_adc *self, int value) {
	return ((struct lm_adc_api *)self)->convert_to_millivolts(self, value);
}

/**
 * @brief Convert a raw sample using the channel's calibration.
 *
 * Channels initialized with lm_adc_channel_init() use the default
 * calibration established by lm_adc_calibrate().
 *
 * @param[in] self The ADC instance.
 * @param[in] channel The channel from which the sample was read.
 * @param[in] value The raw ADC sample.
 *
 * @return The voltage in millivolts on success, -ENOTSUP if the port omits
 *         this operation, or another negative error on failure.
 */
static inline int lm_adc_convert_channel_to_millivolts(struct lm_adc *self,
		lm_adc_channel_t channel, int value) {
	const struct lm_adc_api *api = (const struct lm_adc_api *)self;

	if (!api->convert_to_millivolts_channel) {
		return -ENOTSUP;
	}
	return api->convert_to_millivolts_channel(self, channel, value);
}

struct lm_adc *lm_adc_create(uint8_t adc_num);
int lm_adc_delete(struct lm_adc *self);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_ADC_H */
