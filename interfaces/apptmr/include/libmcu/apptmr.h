/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@libmcu.org>
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
	int (*start)(struct apptmr *self, uint32_t timeout_ms);
	int (*restart)(struct apptmr *self, uint32_t timeout_ms);
	int (*stop)(struct apptmr *self);
	void (*trigger)(struct apptmr *self);
};

static inline int apptmr_enable(struct apptmr *self) {
	return ((struct apptmr_api *)self)->enable(self);
}

static inline int apptmr_disable(struct apptmr *self) {
	return ((struct apptmr_api *)self)->disable(self);
}

static inline int apptmr_start(struct apptmr *self, uint32_t timeout_ms) {
	return ((struct apptmr_api *)self)->start(self, timeout_ms);
}

static inline int apptmr_restart(struct apptmr *self, uint32_t timeout_ms) {
	return ((struct apptmr_api *)self)->restart(self, timeout_ms);
}

static inline int apptmr_stop(struct apptmr *self) {
	return ((struct apptmr_api *)self)->stop(self);
}

static inline void apptmr_trigger(struct apptmr *self) {
	((struct apptmr_api *)self)->trigger(self);
}

struct apptmr *apptmr_create(bool periodic, apptmr_callback_t cb, void *cb_ctx);
int apptmr_delete(struct apptmr *self);

void apptmr_create_hook(struct apptmr *self);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_APPTMR_H */
