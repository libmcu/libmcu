#ifndef LIBMCU_BITMAP_H
#define LIBMCU_BITMAP_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

typedef uintptr_t bitmap_static_t;
typedef bitmap_static_t * bitmap_t;

/** Bitmap unit size in bits, that shows each one in array can hold at most */
#define BITMAP_UNIT_BITS		\
	((int)(sizeof(bitmap_static_t) * CHAR_BIT))
#define BITMAP_DIV(nbits)		((nbits) / BITMAP_UNIT_BITS)
#define BITMAP_REMAIN(nbits)		((nbits) % BITMAP_UNIT_BITS)
#define BITMAP_ARRAY_SIZE(nbits)	\
	(BITMAP_DIV(nbits) + !!BITMAP_REMAIN(nbits))

/**
 * @brief Define a bitmap
 * @param name name of bitmap
 * @param nbits number of bits to use
 */
#define DEFINE_BITMAP(name, nbits)	\
	bitmap_static_t name[BITMAP_ARRAY_SIZE(nbits)]

/**
 * @brief Get a value of bit in the bitmap
 * @param bitmap a pointer to @ref bitmap_t
 * @param pos bit position starting from index 0
 * @return 1 or 0
 */
bool bitmap_get(const bitmap_t bitmap, int pos);

/**
 * @brief Set a bit in the bitmap
 * @param bitmap a pointer to @ref bitmap_t
 * @param pos bit position to set
 */
void bitmap_set(bitmap_t bitmap, int pos);

/**
 * @brief Count number of bits set in the bitmap
 * @param bitmap a pointer to @ref bitmap_t
 * @param n number of bits used
 * @return number of bits set
 */
int bitmap_count(const bitmap_t bitmap, int n);

/**
 * @brief Clear a bit in the bitmap
 * @param bitmap a pointer to @ref bitmap_t
 * @param pos bit position to clear
 */
void bitmap_clear(bitmap_t bitmap, int pos);

/**
 * @brief Initialize bitmap
 * @param bitmap a pointer to @ref bitmap_t
 * @param n number of bits to initialize in the array
 * @param initial_value initial value in the array
 */
void bitmap_create_static(bitmap_static_t * const bitmap,
		int n, bool initial_value);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_BITMAP_H */
