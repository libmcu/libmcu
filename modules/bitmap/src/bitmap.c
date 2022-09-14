/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/bitmap.h"

bool bitmap_get(const bitmap_t bitmap, int pos)
{
	bitmap_static_t val;

	val = bitmap[pos / BITMAP_UNIT_BITS];
	val &= (1UL << (pos % BITMAP_UNIT_BITS));

	return val != 0;
}

void bitmap_set(bitmap_t bitmap, int pos)
{
	bitmap_static_t t;
	int index = pos / BITMAP_UNIT_BITS;

	t = bitmap[index];
	t |= 1UL << (pos % BITMAP_UNIT_BITS);

	bitmap[index] = t;
}

int bitmap_count(const bitmap_t bitmap, int n)
{
	int nword = n / BITMAP_UNIT_BITS;
	int remained = n % BITMAP_UNIT_BITS;
	int cnt = 0;

	for (int i = 0; i < nword; i++) {
		for (int j = 0; (bitmap[i] >> j) &&
				(j < (int)(sizeof(bitmap[i]) * CHAR_BIT)); j++) {
			if ((bitmap[i] >> j) & 1U) {
				cnt++;
			}
		}
	}

	for (int i = 0; (i < remained) && (bitmap[nword] >> i); i++) {
		if ((bitmap[nword] >> i) & 1U)
			cnt++;
	}

	return cnt;
}

void bitmap_clear(bitmap_t bitmap, int pos)
{
	bitmap_static_t t;
	int index = pos / BITMAP_UNIT_BITS;

	t = bitmap[index];
	t &= ~(1UL << (pos % BITMAP_UNIT_BITS));

	bitmap[index] = t;
}

void bitmap_create_static(bitmap_static_t * const bitmap,
		int n, bool initial_value)
{
	int nword = n / BITMAP_UNIT_BITS;
	int remained = n % BITMAP_UNIT_BITS;

	for (int i = 0; i < nword; i++) {
		bitmap[i] = (-1UL * initial_value);
	}

	if (remained) {
		bitmap_static_t t = 0;

		for (int i = 0; i < remained; i++) {
			t = (t << 1UL) | (1UL * initial_value);
		}

		bitmap[nword] = t;
	}
}
