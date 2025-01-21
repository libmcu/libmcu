/*
 * SPDX-FileCopyrightText: 2024 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_STREXT_H
#define LIBMCU_STREXT_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>

/**
 * @typedef strchunk_cb_t
 * @brief Callback function type for processing each chunk of the string.
 *
 * @param[in] chunk Pointer to the chunk of the string.
 * @param[in] chunk_len Length of the chunk.
 * @param[in] ctx User-defined context passed to the callback function.
 */
typedef void (*strchunk_cb_t)(const char *chunk, size_t chunk_len, void *ctx);

/**
 * @brief Splits a string by a delimiter and calls a callback function for each
 * chunk.
 *
 * This function takes an input string and splits it into chunks based on the
 * specified delimiter. For each chunk, the provided callback function is called
 * with the chunk, its length, and a user-defined context.
 *
 * @param[in] str The input string to be split.
 * @param[in] delimiter The character used to split the string.
 * @param[in] cb The callback function to be called for each chunk.
 * @param[in] cb_ctx User-defined context to be passed to the callback function.
 *
 * @return The number of chunks processed.
 */
size_t strchunk(const char *str, const char delimiter,
		strchunk_cb_t cb, void *cb_ctx);

/**
 * @brief Removes the given leading and trailing characters.
 *
 * @param s string to be manipulated
 * @param[in] c a character to be removed
 *
 * @return A pointer to the result string
 */
char *strtrim(char *s, const char c);

/**
 * @brief Converts all characters in the string to uppercase.
 *
 * This function iterates through each character in the input string
 * and converts it to its uppercase equivalent.
 *
 * @param[in,out] s The input string to be converted. The string is modified
 *                in place.
 */
void strupper(char *s);

/**
 * @brief Converts all characters in the string to lowercase.
 *
 * This function iterates through each character in the input string
 * and converts it to its lowercase equivalent.
 *
 * @param[in,out] s The input string to be converted. The string is modified
 *                in place.
 */
void strlower(char *s);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_STREXT_H */
