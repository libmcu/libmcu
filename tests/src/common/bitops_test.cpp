#include "CppUTest/TestHarness.h"
#include "CppUTest/TestHarness_c.h"

#include <string.h>
#include "libmcu/bitops.h"

TEST_GROUP(bitops) {
	void setup(void) {
	}
	void teardown(void) {
	}
};

TEST(bitops, flsl_ShouldReturnZero_WhenZeroGiven) {
	LONGS_EQUAL(0, flsl(0));
}
TEST(bitops, flsl_ShouldReturnOne_WhenOneGiven) {
	LONGS_EQUAL(1, flsl(1));
}
TEST(bitops, flsl_ShouldReturn32_WhenMSBSet) {
	LONGS_EQUAL(32, flsl(0x80000000));
}
TEST(bitops, flsl_ShouldReturn32_WhenMSBSetWithOtherBitSet) {
	LONGS_EQUAL(32, flsl(0x8fffffff));
}

TEST(bitops, is_power2_ShouldReturnFalse_WhenZeroGiven) {
	LONGS_EQUAL(0, is_power2(0));
}
TEST(bitops, is_power2_ShouldReturnTrue_WhenOneGiven) {
	LONGS_EQUAL(1, is_power2(1));
}
TEST(bitops, is_power2_ShouldReturnFalse_WhenOddNumberGiven) {
	LONGS_EQUAL(0, is_power2(3));
	LONGS_EQUAL(0, is_power2(5));
	LONGS_EQUAL(0, is_power2(7));
}
TEST(bitops, is_power2_ShouldReturnFalse_WhenNonPower2EvenNumberGiven) {
	LONGS_EQUAL(0, is_power2(6));
	LONGS_EQUAL(0, is_power2(10));
}
TEST(bitops, is_power2_ShouldReturnTrue_WhenPowerOf2Given) {
	LONGS_EQUAL(1, is_power2(2));
	LONGS_EQUAL(1, is_power2(65536));
}
