/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/gpio.h"
#include "libmcu/port/gpio.h"

struct gpio *gpio_create(uint8_t channel)
{
	return gpio_port_create(channel);
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
