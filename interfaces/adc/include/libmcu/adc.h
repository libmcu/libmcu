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
#define adc_channel_t		libmcu_adc_channel_t

typedef enum {
	ADC_CH_1		= 0x00000001UL,
	ADC_CH_2		= 0x00000002UL,
	ADC_CH_3		= 0x00000004UL,
	ADC_CH_4		= 0x00000008UL,
	ADC_CH_5		= 0x00000010UL,
	ADC_CH_6		= 0x00000020UL,
	ADC_CH_7		= 0x00000040UL,
	ADC_CH_8		= 0x00000080UL,
	ADC_CH_9		= 0x00000100UL,
	ADC_CH_10		= 0x00000200UL,
	ADC_CH_11		= 0x00000400UL,
	ADC_CH_12		= 0x00000800UL,
	ADC_CH_13		= 0x00001000UL,
	ADC_CH_14		= 0x00002000UL,
	ADC_CH_15		= 0x00004000UL,
	ADC_CH_16		= 0x00008000UL,
	ADC_CH_17		= 0x00010000UL,
	ADC_CH_18		= 0x00020000UL,
	ADC_CH_19		= 0x00040000UL,
	ADC_CH_20		= 0x00080000UL,
	ADC_CH_ALL		= 0x7fffffffUL,
} adc_channel_t;

struct adc;

struct adc_api {
	int (*enable)(struct adc *self);
	int (*disable)(struct adc *self);
	int (*init_channel)(struct adc *self, adc_channel_t channel);
	int (*calibrate)(struct adc *self);
	int (*measure)(struct adc *self);
	int (*read)(struct adc *self, adc_channel_t channel);
	int (*convert_to_millivolts)(struct adc *self, int value);
};

static inline int adc_enable(struct adc *self) {
	return ((struct adc_api *)self)->enable(self);
}

static inline int adc_disable(struct adc *self) {
	return ((struct adc_api *)self)->disable(self);
}

static inline int adc_channel_init(struct adc *self, adc_channel_t channel) {
	return ((struct adc_api *)self)->init_channel(self, channel);
}

static inline int adc_calibrate(struct adc *self) {
	return ((struct adc_api *)self)->calibrate(self);
}

static inline int adc_measure(struct adc *self) {
	return ((struct adc_api *)self)->measure(self);
}

static inline int adc_read(struct adc *self, adc_channel_t channel) {
	return ((struct adc_api *)self)->read(self, channel);
}

static inline int adc_convert_to_millivolts(struct adc *self, int value) {
	return ((struct adc_api *)self)->convert_to_millivolts(self, value);
}

struct adc *adc_create(uint8_t adc_num);
int adc_delete(struct adc *self);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_ADC_H */
