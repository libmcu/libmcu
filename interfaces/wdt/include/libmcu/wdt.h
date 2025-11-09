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

struct lm_wdt;

/**
 * @brief Callback function type for watchdog timeout.
 *
 * @param[in] wdt Pointer to the watchdog instance.
 * @param[in] ctx User-defined context for the callback.
 */
typedef void (*lm_wdt_timeout_cb_t)(struct lm_wdt *wdt, void *ctx);

/**
 * @brief Callback function type for iterating over watchdog timers.
 *
 * This callback is invoked for each watchdog timer during iteration.
 *
 * @param[in] wdt Pointer to the current watchdog timer instance.
 * @param[in] ctx User-defined context passed to the callback.
 */
typedef void (*lm_wdt_foreach_cb_t)(struct lm_wdt *wdt, void *ctx);

/**
 * @typedef lm_wdt_periodic_cb_t
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
typedef void (*lm_wdt_periodic_cb_t)(void *ctx);

/**
 * @brief Initialize the watchdog timer module.
 *
 * This function initializes the watchdog timer module. It should be called
 * before any other watchdog timer functions are used. This function sets up
 * the necessary hardware or software resources required for the watchdog
 * timer to operate.
 *
 * @param[in] cb A callback function that is periodically invoked by the
 *               watchdog timer. This can be used to reset the watchdog timer
 *               or perform periodic tasks. Pass NULL if no callback is needed.
 * @param[in] cb_ctx A user-defined context that is passed to the callback
 *                   function. This can be used to provide additional data or
 *                   state to the callback.
 * @param[in] threaded A boolean flag indicating whether the watchdog timer
 *                     should operate in a threaded mode. If true, the watchdog
 *                     timer runs in a separate thread; otherwise, it operates
 *                     in the current execution context.
 *
 * @return 0 on success, negative error code on failure.
 */
int lm_wdt_init(lm_wdt_periodic_cb_t cb, void *cb_ctx, bool threaded);

/**
 * @brief Deinitialize the watchdog timer.
 *
 * This function deinitializes the watchdog timer, releasing any resources
 * that were allocated during initialization. It should be called when the
 * watchdog timer is no longer needed.
 */
void lm_wdt_deinit(void);

/**
 * @brief Start the watchdog timer.
 *
 * This function activates the watchdog timer. Once started, the watchdog
 * timer begins monitoring the system and must be periodically reset to
 * prevent a system reset.
 *
 * @return 0 on success, negative error code on failure.
 */
int lm_wdt_start(void);

/**
 * @brief Stop the watchdog timer.
 *
 * This function deactivates the watchdog timer. After stopping, the watchdog
 * timer will no longer monitor the system, and no resets will occur due to
 * watchdog timer expiration.
 */
void lm_wdt_stop(void);

/**
 * @brief Perform a step operation for the watchdog timer.
 *
 * This function processes all registered task watchdogs and calculates
 * the next deadline for the watchdog timer. The provided variable is updated
 * with the time remaining (in milliseconds) before the next reset is required.
 *
 * @note This function must be called within the specified `next_deadline_ms`
 *       to ensure proper operation. Delays in calling this function may result
 *       in missing short-period task watchdogs or, if limits are exceeded,
 *       the system may reset.
 *
 * @param[out] next_deadline_ms Pointer to a variable where the next deadline
 *                         (in milliseconds) will be stored.
 *
 * @return 0 on success, negative error code on failure.
 */
int lm_wdt_step(uint32_t *next_deadline_ms);

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
int lm_wdt_enable(struct lm_wdt *self);

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
int lm_wdt_disable(struct lm_wdt *self);

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
int lm_wdt_register_timeout_cb(lm_wdt_timeout_cb_t cb, void *cb_ctx);

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
struct lm_wdt *lm_wdt_new(const char *name, const uint32_t period_ms,
		lm_wdt_timeout_cb_t cb, void *cb_ctx);

/**
 * @brief Delete a watchdog timer instance.
 *
 * @param[in] self Pointer to the watchdog timer instance to be deleted.
 */
void lm_wdt_delete(struct lm_wdt *self);

/**
 * @brief Feed (reset) the watchdog timer.
 *
 * @param[in] self Pointer to the watchdog timer instance.
 *
 * @return 0 on success, negative error code on failure.
 */
int lm_wdt_feed(struct lm_wdt *self);

/**
 * @brief Get the name of the watchdog timer.
 *
 * This function returns the name of the specified watchdog timer instance.
 *
 * @param[in] self Pointer to the watchdog timer instance.
 *
 * @return The name of the watchdog timer.
 */
const char *lm_wdt_name(const struct lm_wdt *self);

/**
 * @brief Iterate over all registered watchdog timers.
 *
 * This function invokes the provided callback for each registered
 * watchdog timer.
 *
 * @param[in] cb Callback function to be invoked for each watchdog timer.
 * @param[in] cb_ctx User-defined context to pass to the callback.
 */
void lm_wdt_foreach(lm_wdt_foreach_cb_t cb, void *cb_ctx);

/**
 * @brief Get the timeout period of the specified watchdog timer.
 *
 * @param[in] self Pointer to the watchdog timer instance.
 *
 * @return Timeout period in milliseconds.
 */
uint32_t lm_wdt_get_period(const struct lm_wdt *self);

/**
 * @brief Get the time elapsed since the last feed of the watchdog timer.
 *
 * @param[in] self Pointer to the watchdog timer instance.
 *
 * @return Time in milliseconds since the last feed.
 */
uint32_t lm_wdt_get_time_since_last_feed(const struct lm_wdt *self);

/**
 * @brief Check if the specified watchdog timer is enabled.
 *
 * @param[in] self Pointer to the watchdog timer instance.
 *
 * @return true if the watchdog timer is enabled, false otherwise.
 */
bool lm_wdt_is_enabled(const struct lm_wdt *self);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_WDT_H */
