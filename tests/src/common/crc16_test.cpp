/*
 * SPDX-FileCopyrightText: 2025 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "CppUTest/TestHarness.h"
#include "CppUTest/TestHarness_c.h"

#include "libmcu/crc16.h"

TEST_GROUP(CRC16) {
	void setup(void) {
	}
	void teardown(void) {
	}
};

TEST(CRC16, ShouldComputeModbusCRC) {
	const uint8_t t[] = "123456789";
	LONGS_EQUAL(0x4B37, crc16_modbus(t, sizeof(t)-1));
}
TEST(CRC16, ShouldComputeXmodemCRC) {
	const uint8_t t[] = "123456789";
	LONGS_EQUAL(0x31C3, crc16_xmodem(t, sizeof(t)-1));
}
