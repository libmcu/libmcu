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
};

void fsm_init(struct fsm *fsm, const struct fsm_item *items, size_t item_len,
		void *ctx);
void fsm_step(struct fsm *fsm);
void fsm_reset(struct fsm *fsm);
fsm_state_t fsm_state(struct fsm *fsm);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_FSM_H */
