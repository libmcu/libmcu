/*
 * SPDX-FileCopyrightText: 2024 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/fsm.h"

#if !defined(FSM_INFO)
#define FSM_INFO(...)
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
		if (current_state != next_candidate) {
			*next_state = next_candidate;

			FSM_INFO("FSM state change from %d to %d",
					current_state, next_candidate);
		}

		if (item->action.run) {
			(*item->action.run)(current_state, next_candidate, ctx);
		}

		return true;
	}

	return false;
}

fsm_state_t fsm_step(struct fsm *fsm)
{
	fsm_state_t current_state = fsm->state.present;

	for (size_t i = 0; i < fsm->item_len; i++) {
		const struct fsm_item *item = &fsm->item[i];
		if (process(item, current_state, &fsm->state.present,
				fsm->ctx)) {
			break;
		}
	}

	if (fsm->cb && current_state != fsm->state.present) {
		(*fsm->cb)(fsm, fsm->state.present, current_state, fsm->cb_ctx);
	}

	return fsm->state.present;
}

fsm_state_t fsm_get_state(const struct fsm *fsm)
{
	return fsm->state.present;
}

void fsm_set_state_change_cb(struct fsm *fsm,
		fsm_state_change_cb_t cb, void *cb_ctx)
{
	fsm->cb = cb;
	fsm->cb_ctx = cb_ctx;
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
