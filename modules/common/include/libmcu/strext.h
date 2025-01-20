/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_STREXT_H
#define LIBMCU_STREXT_H

#if defined(__cplusplus)
extern "C" {
#endif

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
