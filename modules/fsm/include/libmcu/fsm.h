/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_FSM_H
#define LIBMCU_FSM_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define FSM_ITEM(present_state, event_func, action_func, next_state) { \
		.state = (struct fsm_state) { present_state, next_state }, \
		.event = event_func, \
		.action = action_func, \
	}

typedef int16_t fsm_state_t;

typedef void (*fsm_handler_t)(fsm_state_t state, fsm_state_t next_state,
		void *ctx);
typedef bool (*fsm_event_t)(fsm_state_t state, fsm_state_t next_state,
		void *ctx);

struct fsm;
typedef void (*fsm_state_change_cb_t)(struct fsm *fsm,
		fsm_state_t new_state, fsm_state_t prev_state, void *ctx);

struct fsm_action {
	fsm_handler_t run;
};

struct fsm_state {
	fsm_state_t present;
	fsm_state_t next;
};

struct fsm_item {
	struct fsm_state state;
	fsm_event_t event;
	struct fsm_action action;
};

struct fsm {
	struct fsm_state state;
	const struct fsm_item *item;
	size_t item_len;
	void *ctx;

	fsm_state_change_cb_t cb;
	void *cb_ctx;
};

/**
 * @brief Initialize the finite state machine (FSM).
 *
 * @param[in] fsm Pointer to the FSM structure to initialize.
 * @param[in] items Array of FSM items (states and transitions).
 * @param[in] item_len Length of the items array.
 * @param[in] ctx Context pointer to be passed to state functions.
 */
void fsm_init(struct fsm *fsm,
		const struct fsm_item *items, size_t item_len, void *ctx);

/**
 * @brief Set the callback for state changes in the finite state machine.
 *
 * This function sets a callback function that will be called whenever the
 * state of the finite state machine (FSM) changes. The callback function
 * will be provided with the FSM instance, the new state, and the user-defined
 * context.
 *
 * @param[in] fsm Pointer to the FSM structure.
 * @param[in] cb The callback function to be called on state changes.
 * @param[in] cb_ctx User-defined context to be passed to the callback function.
 */
void fsm_set_state_change_cb(struct fsm *fsm,
		fsm_state_change_cb_t cb, void *cb_ctx);

/**
 * @brief Perform a single step in the finite state machine.
 *
 * This function advances the finite state machine (FSM) by one step,
 * transitioning to the next state based on the current state and the
 * defined state transitions.
 *
 * @param[in] fsm A pointer to the finite state machine structure.
 *
 * @return The new state of the finite state machine after the step.
 */
fsm_state_t fsm_step(struct fsm *fsm);

/**
 * @brief Reset the FSM to its initial state.
 *
 * @param[in] fsm Pointer to the FSM structure.
 */
void fsm_reset(struct fsm *fsm);

/**
 * @brief Get the current state of the FSM.
 *
 * @param[in] fsm Pointer to the FSM structure.
 *
 * @return The current state of the FSM.
 */
fsm_state_t fsm_get_state(const struct fsm *fsm);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_FSM_H */
