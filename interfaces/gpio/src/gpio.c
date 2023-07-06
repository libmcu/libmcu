/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/gpio.h"
#include "libmcu/port/gpio.h"

struct gpio *gpio_create(uint16_t pin)
{
	return gpio_port_create(pin);
}

void gpio_delete(struct gpio *self)
{
	gpio_port_delete(self);
}

int gpio_enable(struct gpio *self)
{
	return gpio_port_enable(self);
}

int gpio_disable(struct gpio *self)
{
	return gpio_port_disable(self);
}

int gpio_set(struct gpio *self, int value)
{
	return gpio_port_set(self, value);
}

int gpio_get(struct gpio *self)
{
	return gpio_port_get(self);
}

int gpio_register_handler(struct gpio *self, gpio_handler_t handler, void *ctx)
{
	return gpio_port_register_handler(self, handler, ctx);
}
