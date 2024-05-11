/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/fsm.h"

#if !defined(FSM_DEBUG)
#define FSM_DEBUG(...)
#endif

static bool process(const struct fsm_item *item,
		const fsm_state_t current_state,
		fsm_state_t *next_state, void *ctx)
{
	if (item->state.present != current_state) {
		return false;
	}

	fsm_state_t next_candidate = item->state.next;

	if (item->event && (*item->event)(current_state, next_candidate, ctx)) {
		if (item->action.run) {
			(*item->action.run)(current_state, next_candidate, ctx);
		}

		FSM_DEBUG("FSM state change from %d to %d",
				current_state, next_candidate);
		*next_state = next_candidate;
		return true;
	}

	return false;
}

fsm_state_t fsm_state(struct fsm *fsm)
{
	return fsm->state.present;
}

void fsm_step(struct fsm *fsm)
{
	fsm_state_t current_state = fsm->state.present;

	for (size_t i = 0; i < fsm->item_len; i++) {
		const struct fsm_item *item = &fsm->item[i];
		if (process(item, current_state, &fsm->state.present,
				fsm->ctx)) {
			break;
		}
	}
}

void fsm_reset(struct fsm *fsm)
{
	fsm->state = (struct fsm_state) { 0, 0 };
}

void fsm_init(struct fsm *fsm, const struct fsm_item *items, size_t item_len,
		void *ctx)
{
	fsm->item = items;
	fsm->item_len = item_len;
	fsm->ctx = ctx;

	fsm_reset(fsm);
}
