/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_GPIO_PORT_H
#define LIBMCU_GPIO_PORT_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "libmcu/gpio.h"

struct gpio *gpio_port_create(uint8_t channel);
void gpio_port_delete(struct gpio *self);
int gpio_port_enable(struct gpio *self);
int gpio_port_disable(struct gpio *self);
int gpio_port_set(struct gpio *self, int value);
int gpio_port_get(struct gpio *self);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_GPIO_PORT_H */
