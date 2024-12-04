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

struct wdt;

/**
 * @brief Callback function type for watchdog timeout.
 *
 * @param[in] wdt Pointer to the watchdog instance.
 * @param[in] ctx User-defined context for the callback.
 */
typedef void (*wdt_timeout_cb_t)(struct wdt *wdt, void *ctx);

/**
 * @brief Initialize the watchdog timer.
 *
 * @param[in] cb Callback function to be called on watchdog timeout.
 * @param[in] cb_ctx User-defined context to be passed to the callback function.
 *
 * @return 0 on success, negative error code on failure.
 */
int wdt_init(wdt_timeout_cb_t cb, void *cb_ctx);

/**
 * @brief Deinitialize the watchdog timer.
 *
 * This function deinitializes the watchdog timer, releasing any resources
 * that were allocated during initialization. It should be called when the
 * watchdog timer is no longer needed.
 */
void wdt_deinit(void);

/**
 * @brief Create a new watchdog timer instance.
 *
 * @return Pointer to the newly created watchdog timer instance,
 *                 or NULL on failure.
 */
struct wdt *wdt_new(void);

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
