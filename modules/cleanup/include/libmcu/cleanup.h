/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_CLEANUP_H
#define LIBMCU_CLEANUP_H

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Enumeration of possible cleanup errors.
 */
typedef enum {
	CLEANUP_ERROR_NONE,           /**< No error occurred. */
	CLEANUP_ERROR_ALLOC_FAILED,   /**< Allocation failed. */
} cleanup_error_t;

/**
 * @brief Type definition for cleanup function pointer.
 *
 * @param[in] ctx Context pointer to be passed to the cleanup function.
 */
typedef void (*cleanup_func_t)(void *ctx);

/**
 * @brief Initialize the cleanup module.
 *
 * @return CLEANUP_ERROR_NONE on success, or an appropriate error code on
 *         failure.
 */
cleanup_error_t cleanup_init(void);

/**
 * @brief Deinitialize the cleanup module.
 */
void cleanup_deinit(void);

/**
 * @brief Register a cleanup function to be called during cleanup.
 *
 * @param[in] priority The priority of the cleanup function. Higher priority
 *                     functions are called first.
 * @param[in] func The cleanup function to register.
 * @param[in] func_ctx The context to be passed to the cleanup function.
 *
 * @return CLEANUP_ERROR_NONE on success, or an appropriate error code on
 *         failure.
 */
cleanup_error_t cleanup_register(int priority,
		cleanup_func_t func, void *func_ctx);

/**
 * @brief Execute all registered cleanup functions in order of their priority.
 */
void cleanup_execute(void);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_CLEANUP_H */
