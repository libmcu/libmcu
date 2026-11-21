/*
 * SPDX-FileCopyrightText: 2026 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_ESP_IDF_ADC_RANGE_H
#define LIBMCU_ESP_IDF_ADC_RANGE_H

#include <errno.h>
#include <stdint.h>

/* Port limits from the ESP-IDF Programming Guide's ESP32 attenuation table.
 * The measurable range at each attenuation varies by chip. */
#define ADC_0_DB_MAX_MV		950U
#define ADC_2_5_DB_MAX_MV	1250U
#define ADC_6_DB_MAX_MV		1750U
#define ADC_12_DB_MAX_MV	2450U

enum lm_adc_range {
	LM_ADC_RANGE_DEFAULT,
	LM_ADC_RANGE_0_DB,
	LM_ADC_RANGE_2_5_DB,
	LM_ADC_RANGE_6_DB,
	LM_ADC_RANGE_12_DB,
};

static inline int lm_adc_range_from_mv(uint32_t max_input_mv)
{
	if (max_input_mv <= ADC_0_DB_MAX_MV) {
		return LM_ADC_RANGE_0_DB;
	} else if (max_input_mv <= ADC_2_5_DB_MAX_MV) {
		return LM_ADC_RANGE_2_5_DB;
	} else if (max_input_mv <= ADC_6_DB_MAX_MV) {
		return LM_ADC_RANGE_6_DB;
	} else if (max_input_mv <= ADC_12_DB_MAX_MV) {
		return LM_ADC_RANGE_12_DB;
	}

	return -ERANGE;
}

#endif /* LIBMCU_ESP_IDF_ADC_RANGE_H */
