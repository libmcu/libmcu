/*
 * SPDX-FileCopyrightText: 2024 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_WDT_H
#define LIBMCU_WDT_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

struct wdt;

/**
 * @brief Callback function type for watchdog timeout.
 *
 * @param[in] wdt Pointer to the watchdog instance.
 * @param[in] ctx User-defined context for the callback.
 */
typedef void (*wdt_timeout_cb_t)(struct wdt *wdt, void *ctx);

/**
 * @brief Callback function type for iterating over watchdog timers.
 *
 * This callback is invoked for each watchdog timer during iteration.
 *
 * @param[in] wdt Pointer to the current watchdog timer instance.
 * @param[in] ctx User-defined context passed to the callback.
 */
typedef void (*wdt_foreach_cb_t)(struct wdt *wdt, void *ctx);

/**
 * @typedef wdt_periodic_cb_t
 * @brief Type definition for the periodic callback function.
 *
 * This type defines a function pointer for a callback function that is
 * periodically called by the watchdog timer (WDT) module. The callback
 * function is intended to be used for user-defined actions that need to be
 * executed at regular intervals. The smallest timeout period of all registered
 * watchdog timers is used as the interval for the periodic callback function.
 *
 * @param ctx A pointer to user-defined context data that will be passed to
 *            the callback function when it is called.
 */
typedef void (*wdt_periodic_cb_t)(void *ctx);

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
int wdt_init(wdt_periodic_cb_t cb, void *cb_ctx);

/**
 * @brief Deinitialize the watchdog timer.
 *
 * This function deinitializes the watchdog timer, releasing any resources
 * that were allocated during initialization. It should be called when the
 * watchdog timer is no longer needed.
 */
void wdt_deinit(void);

/**
 * @brief Enable the watchdog timer.
 *
 * This function enables the watchdog timer for the specified instance.
 * Once enabled, the watchdog timer will start monitoring the system and
 * will trigger a timeout if not fed within the specified period.
 *
 * @param[in] self Pointer to the watchdog timer instance.
 *
 * @return 0 on success, negative error code on failure.
 */
int wdt_enable(struct wdt *self);

/**
 * @brief Disable the watchdog timer.
 *
 * This function disables the watchdog timer for the specified instance.
 * Once disabled, the watchdog timer will stop monitoring the system and
 * will not trigger a timeout.
 *
 * @param[in] self Pointer to the watchdog timer instance.
 * @return 0 on success, negative error code on failure.
 */
int wdt_disable(struct wdt *self);

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
 * This function creates a new watchdog timer instance with the specified name,
 * timeout period, and callback function. The watchdog timer will trigger the
 * callback function if the timeout period elapses without being reset.
 *
 * @param[in] name The name of the watchdog timer instance.
 * @param[in] period_ms The timeout period in milliseconds.
 * @param[in] cb The callback function to be called when the timeout period
 *            elapses.
 * @param[in] cb_ctx A user-defined context pointer that will be passed to
 *            the callback function.
 *
 * @return A pointer to the newly created watchdog timer instance,
 *         or NULL on failure.
 */
struct wdt *wdt_new(const char *name, const uint32_t period_ms,
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

/**
 * @brief Get the name of the watchdog timer.
 *
 * This function returns the name of the specified watchdog timer instance.
 *
 * @param[in] self Pointer to the watchdog timer instance.
 *
 * @return The name of the watchdog timer.
 */
const char *wdt_name(const struct wdt *self);

/**
 * @brief Iterate over all registered watchdog timers.
 *
 * This function invokes the provided callback for each registered
 * watchdog timer.
 *
 * @param[in] cb Callback function to be invoked for each watchdog timer.
 * @param[in] cb_ctx User-defined context to pass to the callback.
 */
void wdt_foreach(wdt_foreach_cb_t cb, void *cb_ctx);

/**
 * @brief Get the timeout period of the specified watchdog timer.
 *
 * @param[in] self Pointer to the watchdog timer instance.
 *
 * @return Timeout period in milliseconds.
 */
uint32_t wdt_get_period(const struct wdt *self);

/**
 * @brief Get the time elapsed since the last feed of the watchdog timer.
 *
 * @param[in] self Pointer to the watchdog timer instance.
 *
 * @return Time in milliseconds since the last feed.
 */
uint32_t wdt_get_time_since_last_feed(const struct wdt *self);

/**
 * @brief Check if the specified watchdog timer is enabled.
 *
 * @param[in] self Pointer to the watchdog timer instance.
 *
 * @return true if the watchdog timer is enabled, false otherwise.
 */
bool wdt_is_enabled(const struct wdt *self);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_WDT_H */
