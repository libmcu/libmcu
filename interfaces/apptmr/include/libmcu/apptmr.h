/*
 * SPDX-FileCopyrightText: 2022 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_APPTMR_H
#define LIBMCU_APPTMR_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

struct apptmr;

typedef void (*apptmr_callback_t)(struct apptmr *self, void *ctx);

struct apptmr_api {
	int (*enable)(struct apptmr *self);
	int (*disable)(struct apptmr *self);
	int (*start)(struct apptmr *self, const uint32_t timeout_ms);
	int (*restart)(struct apptmr *self, const uint32_t timeout_ms);
	int (*stop)(struct apptmr *self);
	void (*trigger)(struct apptmr *self);
};

/**
 * @brief Enable the application timer.
 *
 * This function enables the specified application timer instance.
 *
 * @param[in] self Pointer to the application timer instance.
 *
 * @return 0 on success, negative error code on failure.
 */
static inline int apptmr_enable(struct apptmr *self) {
	return ((struct apptmr_api *)self)->enable(self);
}

/**
 * @brief Disable the application timer.
 *
 * This function disables the specified application timer instance.
 *
 * @param[in] self Pointer to the application timer instance.
 *
 * @return 0 on success, negative error code on failure.
 */
static inline int apptmr_disable(struct apptmr *self) {
	return ((struct apptmr_api *)self)->disable(self);
}

/**
 * @brief Start the application timer.
 *
 * This function starts the specified application timer instance with the given timeout.
 *
 * @param[in] self Pointer to the application timer instance.
 * @param[in] timeout_ms Timeout value in milliseconds.
 * @return 0 on success, negative error code on failure.
 */
static inline int apptmr_start(struct apptmr *self, const uint32_t timeout_ms) {
	return ((struct apptmr_api *)self)->start(self, timeout_ms);
}

/**
 * @brief Restart the application timer.
 *
 * This function restarts the specified application timer instance with the given timeout.
 *
 * @param[in] self Pointer to the application timer instance.
 * @param[in] timeout_ms Timeout value in milliseconds.
 *
 * @return 0 on success, negative error code on failure.
 */
static inline int apptmr_restart(struct apptmr *self,
		const uint32_t timeout_ms) {
	return ((struct apptmr_api *)self)->restart(self, timeout_ms);
}

/**
 * @brief Stop the application timer.
 *
 * This function stops the specified application timer instance.
 *
 * @param[in] self Pointer to the application timer instance.
 *
 * @return 0 on success, negative error code on failure.
 */
static inline int apptmr_stop(struct apptmr *self) {
	return ((struct apptmr_api *)self)->stop(self);
}

/**
 * @brief Trigger the application timer.
 *
 * This function triggers the specified application timer instance.
 *
 * @param[in] self Pointer to the application timer instance.
 */
static inline void apptmr_trigger(struct apptmr *self) {
	((struct apptmr_api *)self)->trigger(self);
}

/**
 * @brief Create a new application timer.
 *
 * This function creates a new application timer instance.
 *
 * @param[in] periodic Boolean indicating if the timer is periodic.
 * @param[in] cb Callback function to be called on timer events.
 * @param[in] cb_ctx Context to be passed to the callback function.
 *
 * @return Pointer to the created application timer instance.
 */
struct apptmr *apptmr_create(bool periodic, apptmr_callback_t cb, void *cb_ctx);

/**
 * @brief Delete the application timer.
 *
 * This function deletes the specified application timer instance.
 *
 * @param[in] self Pointer to the application timer instance.
 *
 * @return 0 on success, negative error code on failure.
 */
int apptmr_delete(struct apptmr *self);

/**
 * @brief Get the application timer capacity.
 *
 * This function returns the number of application timers that can be created.
 *
 * @return The application timer capacity.
 */
int apptmr_cap(void);

/**
 * @brief Get the application timer length.
 *
 * This function returns the number of application timers that have been
 * created.
 *
 * @return The application timer length.
 */
int apptmr_len(void);

/**
 * @brief Global timeout pre hook function for application timer.
 *
 * This function is called whenever any registered application timer
 * reaches its timeout. It serves as a common hook that can be used to
 * perform actions that need to be executed whenever any timer expires.
 * This function is called before the global timeout callback is executed.
 *
 * @param self Pointer to the application timer instance that triggered
 *             the timeout.
 */
void apptmr_global_pre_timeout_hook(struct apptmr *self);

/**
 * @brief Global timeout post hook function for application timer.
 *
 * This function is called whenever any registered application timer
 * reaches its timeout. It serves as a common hook that can be used to
 * perform actions that need to be executed whenever any timer expires.
 * This function is called after the global timeout callback is executed.
 *
 * @param self Pointer to the application timer instance that triggered
 *             the timeout.
 */
void apptmr_global_post_timeout_hook(struct apptmr *self);

#if defined(UNIT_TEST)
/**
 * @brief Hook function for unit testing application timer creation.
 *
 * This function is used as a hook for unit tests to perform additional
 * operations when an application timer is created.
 *
 * @param self Pointer to the application timer instance.
 */
void apptmr_create_hook(struct apptmr *self);
#endif

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_APPTMR_H */
