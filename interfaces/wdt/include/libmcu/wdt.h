/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_WDT_H
#define LIBMCU_WDT_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>

struct wdt;

/**
 * @brief Callback function type for watchdog timeout.
 *
 * @param[in] wdt Pointer to the watchdog instance.
 * @param[in] ctx User-defined context for the callback.
 */
typedef void (*wdt_timeout_cb_t)(struct wdt *wdt, void *ctx);

/**
 * @brief Initialize the watchdog timer module.
 *
 * This function initializes the watchdog timer module. It should be called
 * before any other watchdog timer functions are used. This function sets up
 * the necessary hardware or software resources required for the watchdog
 * timer to operate.
 *
 * @return 0 on success, negative error code on failure.
 */
int wdt_init(void);

/**
 * @brief Deinitialize the watchdog timer.
 *
 * This function deinitializes the watchdog timer, releasing any resources
 * that were allocated during initialization. It should be called when the
 * watchdog timer is no longer needed.
 */
void wdt_deinit(void);

/**
 * @brief Register a callback function for watchdog timeout.
 *
 * This function registers a callback function that will be called when the
 * watchdog timer times out. The callback function can be used to perform
 * necessary actions before the system resets.
 *
 * @param[in] cb Callback function to be called on watchdog timeout.
 * @param[in] cb_ctx User-defined context to be passed to the callback function.
 *
 * @note The callback registered here will be called when any of the watchdog
 *       timers times out.
 * @note Only one callback can be registered at a time.
 *
 * @return 0 on success, negative error code on failure.
 */
int wdt_register_timeout_cb(wdt_timeout_cb_t cb, void *cb_ctx);

/**
 * @brief Create a new watchdog timer instance.
 *
 * @param[in] period_ms The timeout period in milliseconds for the watchdog
 *                      timer.
 * @param[in] cb Callback function to be called on watchdog timeout.
 * @param[in] cb_ctx User-defined context to be passed to the callback function.
 *
 * @note The callback registered here will be called when this watchdog timer
 *       times out and then the callback registered in wdt_init() will be
 *       called.
 *
 * @return Pointer to the newly created watchdog timer instance,
 *                 or NULL on failure.
 */
struct wdt *wdt_new(const uint32_t period_ms,
		wdt_timeout_cb_t cb, void *cb_ctx);

/**
 * @brief Delete a watchdog timer instance.
 *
 * @param[in] self Pointer to the watchdog timer instance to be deleted.
 */
void wdt_delete(struct wdt *self);

/**
 * @brief Feed (reset) the watchdog timer.
 *
 * @param[in] self Pointer to the watchdog timer instance.
 *
 * @return 0 on success, negative error code on failure.
 */
int wdt_feed(struct wdt *self);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_WDT_H */
