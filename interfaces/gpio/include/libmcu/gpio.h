/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@libmcu.org>
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

struct gpio;

typedef void (*gpio_callback_t)(struct gpio *gpio, void *ctx);

struct gpio_api {
	int (*enable)(struct gpio *self);
	int (*disable)(struct gpio *self);
	int (*set)(struct gpio *self, int value);
	int (*get)(struct gpio *self);
	int (*register_callback)(struct gpio *self,
			gpio_callback_t cb, void *cb_ctx);
};

static inline int gpio_enable(struct gpio *self) {
	return ((struct gpio_api *)self)->enable(self);
}

static inline int gpio_disable(struct gpio *self) {
	return ((struct gpio_api *)self)->disable(self);
}

static inline int gpio_set(struct gpio *self, int value) {
	return ((struct gpio_api *)self)->set(self, value);
}

static inline int gpio_get(struct gpio *self) {
	return ((struct gpio_api *)self)->get(self);
}

static inline int gpio_register_callback(struct gpio *self,
		gpio_callback_t cb, void *cb_ctx) {
	return ((struct gpio_api *)self)->register_callback(self, cb, cb_ctx);
}

struct gpio *gpio_create(uint16_t pin);
void gpio_delete(struct gpio *self);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_GPIO_H */
