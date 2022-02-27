#include "CppUTest/TestHarness.h"
#include "CppUTest/TestHarness_c.h"

#include <string.h>
#include "libmcu/cobs.h"

TEST_GROUP(COBS) {
	void setup(void) {
	}
	void teardown(void) {
	}
};

TEST(COBS, encode_ShouldReturnEncodedSize2_WhenOneZeroByteGiven) {
	uint8_t zero = 0;
	uint8_t expected[] = { 0x01, 0x01, 0x00 };
	uint8_t actual[sizeof(expected)];
	LONGS_EQUAL(2, cobs_encode(actual, sizeof(actual), &zero, sizeof(zero)));
	MEMCMP_EQUAL(expected, actual, sizeof(expected));
}

TEST(COBS, encode_ShouldReturnEncodedSize3_WhenTwoZeroByteGiven) {
	uint8_t data[] = { 0x00, 0x00 };
	uint8_t expected[] = { 0x01, 0x01, 0x01, 0x00 };
	uint8_t actual[sizeof(expected)];
	LONGS_EQUAL(3, cobs_encode(actual, sizeof(actual), &data, sizeof(data)));
	MEMCMP_EQUAL(expected, actual, sizeof(expected));
}

TEST(COBS, encode_ShouldReplaceZeroValueWithOverhead_WhenZeroValueInMiddleGiven) {
	uint8_t data[] = { 0x11, 0x22, 0x00, 0x33 };
	uint8_t expected[] = { 0x03, 0x11, 0x22, 0x02, 0x33, 0x00 };
	uint8_t actual[sizeof(expected)];
	LONGS_EQUAL(5, cobs_encode(actual, sizeof(actual), &data, sizeof(data)));
	MEMCMP_EQUAL(expected, actual, sizeof(expected));
}

TEST(COBS, encode_ShouldReplaceZeroValueWithOverhead_WhenZeroValuesTail) {
	uint8_t data[] = { 0x11, 0x00, 0x00, 0x00 };
	uint8_t expected[] = { 0x02, 0x11, 0x01, 0x01, 0x01, 0x00 };
	uint8_t actual[sizeof(expected)];
	LONGS_EQUAL(5, cobs_encode(actual, sizeof(actual), &data, sizeof(data)));
	MEMCMP_EQUAL(expected, actual, sizeof(expected));
}

TEST(COBS, encode_ShouldReturnEncodedSize_WhenNonZeroVauleGiven) {
	uint8_t data[] = { 0x11, 0x22, 0x33, 0x44 };
	uint8_t expected[] = { 0x05, 0x11, 0x22, 0x33, 0x44, 0x00 };
	uint8_t actual[sizeof(expected)];
	LONGS_EQUAL(5, cobs_encode(actual, sizeof(actual), &data, sizeof(data)));
	MEMCMP_EQUAL(expected, actual, sizeof(expected));
}

TEST(COBS, encode_ShouldReturnEncodedSize_When254BytesDataGiven) {
	uint8_t data[0xfe] = { 0, };
	uint8_t expected[0x100] = { 0xFF, };
	for (size_t i = 0; i < sizeof(data); i++) {
		data[i] = (uint8_t)(i + 1);
		expected[i+1] = (uint8_t)(i + 1);
	}
	uint8_t actual[sizeof(expected)];
	LONGS_EQUAL(0xff, cobs_encode(actual, sizeof(actual), &data, sizeof(data)));
	MEMCMP_EQUAL(expected, actual, sizeof(expected));
}

TEST(COBS, encode_ShouldReturnEncodedSize_When255BytesDataWithLeadingZeroGiven) {
	uint8_t data[0xff] = { 0, };
	uint8_t expected[258] = { 0x01, 0xFF, };
	for (size_t i = 0; i < sizeof(data); i++) {
		data[i] = (uint8_t)i;
		expected[i+2] = (uint8_t)(i + 1);
	}
	expected[256] = 0x00;
	uint8_t actual[sizeof(expected)];
	LONGS_EQUAL(256, cobs_encode(actual, sizeof(actual), &data, sizeof(data)));
	MEMCMP_EQUAL(expected, actual, sizeof(expected));
}

TEST(COBS, encode_ShouldReturnEncodedSize_When255BytesDataGiven) {
	uint8_t data[0xff] = { 0, };
	uint8_t expected[258] = { 0xFF, };
	for (size_t i = 0; i < sizeof(data); i++) {
		data[i] = (uint8_t)(i + 1);
		expected[i+1] = (uint8_t)(i + 1);
	}
	expected[255] = 0x02;
	expected[256] = 0xff;
	uint8_t actual[sizeof(expected)];
	LONGS_EQUAL(257, cobs_encode(actual, sizeof(actual), &data, sizeof(data)));
	MEMCMP_EQUAL(expected, actual, sizeof(expected));
}

TEST(COBS, encode_ShouldReturnEncodedSize_When255BytesDataWithZeroTailGiven) {
	uint8_t data[0xff] = { 0, };
	uint8_t expected[258] = { 0xFF, };
	for (size_t i = 0; i < sizeof(data); i++) {
		data[i] = (uint8_t)(i + 2);
		expected[i+1] = (uint8_t)(i + 2);
	}
	expected[255] = 0x01;
	expected[256] = 0x01;
	uint8_t actual[sizeof(expected)];
	LONGS_EQUAL(257, cobs_encode(actual, sizeof(actual), &data, sizeof(data)));
	MEMCMP_EQUAL(expected, actual, sizeof(expected));
}

TEST(COBS, encode_ShouldReturnEncodedSize_When255BytesDataWithZeroValueGiven) {
	uint8_t data[0xff] = { 0, };
	uint8_t expected[257] = { 0xFE, };
	for (size_t i = 0; i < sizeof(data); i++) {
		data[i] = (uint8_t)(i + 3);
		expected[i+1] = (uint8_t)(i + 3);
	}
	expected[254] = 0x02;
	expected[255] = 0x01;
	uint8_t actual[sizeof(expected)];
	LONGS_EQUAL(256, cobs_encode(actual, sizeof(actual), &data, sizeof(data)));
	MEMCMP_EQUAL(expected, actual, sizeof(expected));
}

TEST(COBS, decode_ShouldReturnZero_WhenOneZeroByteEncodedGiven) {
	uint8_t encoded[] = { 0x01, 0x01, 0x00 };
	uint8_t expected[] = { 0x00 };
	uint8_t actual[sizeof(expected)];
	LONGS_EQUAL(sizeof(expected),
	     cobs_decode(actual, sizeof(actual), encoded, sizeof(encoded)));
	MEMCMP_EQUAL(expected, actual, sizeof(expected));
}

TEST(COBS, decode_ShouldReturnTwoZero) {
	uint8_t encoded[] = { 0x01, 0x01, 0x01, 0x00 };
	uint8_t expected[] = { 0x00, 0x00 };
	uint8_t actual[sizeof(expected)];
	LONGS_EQUAL(sizeof(expected),
	     cobs_decode(actual, sizeof(actual), encoded, sizeof(encoded)));
	MEMCMP_EQUAL(expected, actual, sizeof(expected));
}

TEST(COBS, decode_1) {
	uint8_t encoded[] = { 0x03, 0x11, 0x22, 0x02, 0x33, 0x00 };
	uint8_t expected[] = { 0x11, 0x22, 0x00, 0x33 };
	uint8_t actual[sizeof(expected)];
	LONGS_EQUAL(sizeof(expected),
	     cobs_decode(actual, sizeof(actual), encoded, sizeof(encoded)));
	MEMCMP_EQUAL(expected, actual, sizeof(expected));
}

TEST(COBS, decode_2) {
	uint8_t encoded[] = { 0x05, 0x11, 0x22, 0x33, 0x44, 0x00 };
	uint8_t expected[] = { 0x11, 0x22, 0x33, 0x44 };
	uint8_t actual[sizeof(expected)];
	LONGS_EQUAL(sizeof(expected),
	     cobs_decode(actual, sizeof(actual), encoded, sizeof(encoded)));
	MEMCMP_EQUAL(expected, actual, sizeof(expected));
}

TEST(COBS, decode_3) {
	uint8_t encoded[] = { 0x02, 0x11, 0x01, 0x01, 0x01, 0x00 };
	uint8_t expected[] = { 0x11, 0x00, 0x00, 0x00 };
	uint8_t actual[sizeof(expected)];
	LONGS_EQUAL(sizeof(expected),
	     cobs_decode(actual, sizeof(actual), encoded, sizeof(encoded)));
	MEMCMP_EQUAL(expected, actual, sizeof(expected));
}

TEST(COBS, decode_4) {
	uint8_t encoded[256];
	uint8_t expected[254];
	for (uint8_t i = 1; i < 0xff; i++) {
		encoded[i] = i;
		expected[i-1] = i;
	}
	encoded[0] = 0xff;
	uint8_t actual[sizeof(expected)];
	LONGS_EQUAL(sizeof(expected),
	     cobs_decode(actual, sizeof(actual), encoded, sizeof(encoded)));
	MEMCMP_EQUAL(expected, actual, sizeof(expected));
}

TEST(COBS, decode_5) {
	uint8_t encoded[257];
	uint8_t expected[255] = { 0, };
	for (uint8_t i = 2; i < 0xff; i++) {
		encoded[i] = i;
		expected[i-1] = i;
	}
	encoded[0] = 0x01;
	encoded[1] = 0xff;
	uint8_t actual[sizeof(expected)];
	LONGS_EQUAL(sizeof(expected),
	     cobs_decode(actual, sizeof(actual), encoded, sizeof(encoded)));
	MEMCMP_EQUAL(expected, actual, sizeof(expected));
}

TEST(COBS, decode_6) {
	uint8_t encoded[257];
	uint8_t expected[255] = { 0, };
	for (uint8_t i = 1; i < 0xff; i++) {
		encoded[i] = i;
		expected[i-1] = i;
	}
	encoded[0] = 0xff;
	encoded[0xff] = 0x02;
	encoded[0x100] = 0xff;
	expected[0xfe] = 0xff;
	uint8_t actual[sizeof(expected)];
	LONGS_EQUAL(sizeof(expected),
	     cobs_decode(actual, sizeof(actual), encoded, sizeof(encoded)));
	MEMCMP_EQUAL(expected, actual, sizeof(expected));
}

TEST(COBS, decode_7) {
	uint8_t encoded[257];
	uint8_t expected[255] = { 0, };
	for (uint8_t i = 1; i < 0xff; i++) {
		encoded[i] = (uint8_t)(i + 1);
		expected[i-1] = (uint8_t)(i + 1);
	}
	encoded[0] = 0xff;
	encoded[0xff] = 0x01;
	encoded[0x100] = 0x01;
	expected[0xfe] = 0x00;
	uint8_t actual[sizeof(expected)];
	LONGS_EQUAL(sizeof(expected),
	     cobs_decode(actual, sizeof(actual), encoded, sizeof(encoded)));
	MEMCMP_EQUAL(expected, actual, sizeof(expected));
}

TEST(COBS, decode_8) {
	uint8_t encoded[257];
	uint8_t expected[255] = { 0, };
	for (uint8_t i = 1; i < 0xff; i++) {
		encoded[i] = (uint8_t)(i + 2);
		expected[i-1] = (uint8_t)(i + 2);
	}
	encoded[0] = 0xfe;
	encoded[0xfe] = 0x02;
	encoded[0xff] = 0x01;
	expected[0xfe] = 0x01;
	uint8_t actual[sizeof(expected)];
	LONGS_EQUAL(sizeof(expected),
	     cobs_decode(actual, sizeof(actual), encoded, sizeof(encoded)));
	MEMCMP_EQUAL(expected, actual, sizeof(expected));
}
