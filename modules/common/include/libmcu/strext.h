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

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_STREXT_H */
