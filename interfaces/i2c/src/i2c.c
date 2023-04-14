/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/i2c.h"
#include "libmcu/port/i2c.h"

struct i2c *i2c_create(uint8_t channel)
{
	return i2c_port_create(channel);
}

void i2c_delete(struct i2c *self)
{
	i2c_port_delete(self);
}

int i2c_enable(struct i2c *self)
{
	return i2c_port_enable(self);
}

int i2c_disable(struct i2c *self)
{
	return i2c_port_disable(self);
}

int i2c_write(struct i2c *self, uint8_t addr, uint8_t reg,
		const void *data, size_t data_len)
{
	return i2c_port_write(self, addr, reg, data, data_len);
}

int i2c_read(struct i2c *self, uint8_t addr, uint8_t reg,
		void *buf, size_t bufsize)
{
	return i2c_port_read(self, addr, reg, buf, bufsize);
}
