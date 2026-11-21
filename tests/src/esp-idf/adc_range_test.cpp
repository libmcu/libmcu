/*
 * SPDX-FileCopyrightText: 2026 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "CppUTest/TestHarness.h"
#include "adc_range.h"

TEST_GROUP(AdcRange) {
};

TEST(AdcRange, SelectsSmallestRangeCoveringInput) {
	LONGS_EQUAL(LM_ADC_RANGE_0_DB, lm_adc_range_from_mv(0));
	LONGS_EQUAL(LM_ADC_RANGE_0_DB, lm_adc_range_from_mv(950));
	LONGS_EQUAL(LM_ADC_RANGE_2_5_DB, lm_adc_range_from_mv(951));
	LONGS_EQUAL(LM_ADC_RANGE_2_5_DB, lm_adc_range_from_mv(1250));
	LONGS_EQUAL(LM_ADC_RANGE_6_DB, lm_adc_range_from_mv(1251));
	LONGS_EQUAL(LM_ADC_RANGE_6_DB, lm_adc_range_from_mv(1750));
	LONGS_EQUAL(LM_ADC_RANGE_12_DB, lm_adc_range_from_mv(1751));
	LONGS_EQUAL(LM_ADC_RANGE_12_DB, lm_adc_range_from_mv(2450));
}

TEST(AdcRange, RejectsInputAboveLargestRange) {
	LONGS_EQUAL(-ERANGE, lm_adc_range_from_mv(2451));
	LONGS_EQUAL(-ERANGE, lm_adc_range_from_mv(UINT32_MAX));
}
