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

typedef void (*gpio_handler_t)(struct gpio *gpio, void *ctx);

struct gpio *gpio_create(uint16_t pin);
void gpio_delete(struct gpio *self);
int gpio_enable(struct gpio *self);
int gpio_disable(struct gpio *self);
int gpio_set(struct gpio *self, int value);
int gpio_get(struct gpio *self);
int gpio_register_handler(struct gpio *self, gpio_handler_t handler, void *ctx);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_GPIO_H */
