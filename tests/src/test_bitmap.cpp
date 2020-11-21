#include "CppUTest/TestHarness.h"
#include <string.h>
#include "libmcu/bitmap.h"
#include "libmcu/compiler.h"

#define DEFAULT_BITMAP_LENGTH		100 // bits

TEST_GROUP(bitmap) {
	DEFINE_BITMAP(bitmap, DEFAULT_BITMAP_LENGTH);

	void setup(void) {
		bitmap_init(bitmap, DEFAULT_BITMAP_LENGTH, 0);
	}
	void teardown(void) {
	}
};

TEST(bitmap, define_ShouldPreserveRequiredBytes_WhenNumberOfBitsGiven) {
	DEFINE_BITMAP(arr1, 128);
	LONGS_EQUAL(sizeof(arr1), 128/CHAR_BIT);
	DEFINE_BITMAP(arr2, 5);
	LONGS_EQUAL(sizeof(arr2), sizeof(bitmap_t)); // minimum size
	DEFINE_BITMAP(arr3, 100);
	LONGS_EQUAL(sizeof(arr3), ALIGN(100/CHAR_BIT, sizeof(bitmap_t)));
	DEFINE_BITMAP(arr4, 200);
	LONGS_EQUAL(sizeof(arr4), ALIGN(200/CHAR_BIT, sizeof(bitmap_t)));
}

TEST(bitmap, init_ShouldClearArray_WhenInitialValueIsZero) {
	bitmap_t fixed_bitmap[BITMAP_ARRAY_SIZE(DEFAULT_BITMAP_LENGTH)] = { 0, };
	DEFINE_BITMAP(arr, DEFAULT_BITMAP_LENGTH);

	bitmap_init(arr, DEFAULT_BITMAP_LENGTH, 0);

	MEMCMP_EQUAL(fixed_bitmap, arr, sizeof(fixed_bitmap));
}

TEST(bitmap, init_ShouldSetArray_WhenInitialValueIsOne) {
	bitmap_t fixed_bitmap[BITMAP_ARRAY_SIZE(DEFAULT_BITMAP_LENGTH)] = { 0, };
	memset(fixed_bitmap, -1, DEFAULT_BITMAP_LENGTH/CHAR_BIT);
	for (int i = 0; i < DEFAULT_BITMAP_LENGTH % CHAR_BIT; i++) {
		int pos = DEFAULT_BITMAP_LENGTH - i - 1;
		bitmap_set(fixed_bitmap, pos);
	}
	DEFINE_BITMAP(arr, DEFAULT_BITMAP_LENGTH);

	bitmap_init(arr, DEFAULT_BITMAP_LENGTH, 1);

	LONGS_EQUAL(DEFAULT_BITMAP_LENGTH,
			bitmap_count(arr, DEFAULT_BITMAP_LENGTH));
	MEMCMP_EQUAL(fixed_bitmap, arr, sizeof(fixed_bitmap));
}

TEST(bitmap, set_ShouldSet_WhenTargetBitIsClear) {
	LONGS_EQUAL(false, bitmap_get(bitmap, 73));
	bitmap_set(bitmap, 73);
	LONGS_EQUAL(true, bitmap_get(bitmap, 73));
}

TEST(bitmap, set_ShouldKeepSet_WhenTargetBitIsSet) {
	bitmap_set(bitmap, 4);
	LONGS_EQUAL(true, bitmap_get(bitmap, 4));
	bitmap_set(bitmap, 4);
	LONGS_EQUAL(true, bitmap_get(bitmap, 4));
	LONGS_EQUAL(0x10, bitmap[0]);
}

TEST(bitmap, clear_ShouldClear_WhenTargetBitIsSet) {
	bitmap_set(bitmap, 0);
	LONGS_EQUAL(true, bitmap_get(bitmap, 0));
	bitmap_clear(bitmap, 0);
	LONGS_EQUAL(false, bitmap_get(bitmap, 0));
	LONGS_EQUAL(0, bitmap[0]);
}

TEST(bitmap, clear_ShouldKeepClear_WhenTargetBitIsClear) {
	LONGS_EQUAL(false, bitmap_get(bitmap, 0));
	bitmap_clear(bitmap, 0);
	LONGS_EQUAL(false, bitmap_get(bitmap, 0));
}
