/*
 * SPDX-FileCopyrightText: 2022 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_GPIO_H
#define LIBMCU_GPIO_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

struct lm_gpio;

typedef void (*lm_gpio_callback_t)(struct lm_gpio *gpio, void *ctx);

struct lm_gpio_api {
	int (*enable)(struct lm_gpio *self);
	int (*disable)(struct lm_gpio *self);
	int (*enable_interrupt)(struct lm_gpio *self);
	int (*disable_interrupt)(struct lm_gpio *self);
	int (*set)(struct lm_gpio *self, int value);
	int (*get)(struct lm_gpio *self);
	int (*register_callback)(struct lm_gpio *self,
			lm_gpio_callback_t cb, void *cb_ctx);
};

static inline int lm_gpio_enable(struct lm_gpio *self) {
	return ((struct lm_gpio_api *)self)->enable(self);
}

static inline int lm_gpio_disable(struct lm_gpio *self) {
	return ((struct lm_gpio_api *)self)->disable(self);
}

static inline int lm_gpio_enable_interrupt(struct lm_gpio *self) {
	return ((struct lm_gpio_api *)self)->enable_interrupt(self);
}

static inline int lm_gpio_disable_interrupt(struct lm_gpio *self) {
	return ((struct lm_gpio_api *)self)->disable_interrupt(self);
}

static inline int lm_gpio_set(struct lm_gpio *self, int value) {
	return ((struct lm_gpio_api *)self)->set(self, value);
}

static inline int lm_gpio_get(struct lm_gpio *self) {
	return ((struct lm_gpio_api *)self)->get(self);
}

static inline int lm_gpio_register_callback(struct lm_gpio *self,
		lm_gpio_callback_t cb, void *cb_ctx) {
	return ((struct lm_gpio_api *)self)->register_callback(self, cb, cb_ctx);
}

struct lm_gpio *lm_gpio_create(uint16_t pin);
void lm_gpio_delete(struct lm_gpio *self);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_GPIO_H */
