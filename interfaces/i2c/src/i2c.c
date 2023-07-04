/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/i2c.h"
#include "libmcu/port/i2c.h"

#include <errno.h>

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
	if (self->enabled) {
		return -EALREADY;
	}

	int err = i2c_port_enable(self);

	if (!err) {
		self->enabled = true;
	}

	return err;
}

int i2c_disable(struct i2c *self)
{
	if (!self->enabled) {
		return -ENODEV;
	}

	int err = i2c_port_disable(self);
	self->enabled = false;

	return err;
}

int i2c_read(struct i2c *self, uint8_t slave_addr,
		void *buf, size_t bufsize, uint32_t timeout_ms)
{
	if (!self->enabled) {
		return -ENODEV;
	}

	return i2c_port_read(self, slave_addr, buf, bufsize, timeout_ms);
}

int i2c_write(struct i2c *self, uint8_t slave_addr,
		const void *data, size_t data_len, uint32_t timeout_ms)
{
	if (!self->enabled) {
		return -ENODEV;
	}

	return i2c_port_write(self, slave_addr, data, data_len, timeout_ms);
}

int i2c_read_reg(struct i2c *self, uint8_t slave_addr,
		uint32_t reg_addr, uint8_t reg_addr_bits,
		void *buf, size_t bufsize, uint32_t timeout_ms)
{
	if (!self->enabled) {
		return -ENODEV;
	}

	return i2c_port_read_reg(self, slave_addr, reg_addr, reg_addr_bits,
			buf, bufsize, timeout_ms);
}

int i2c_write_reg(struct i2c *self, uint8_t slave_addr,
		uint32_t reg_addr, uint8_t reg_addr_bits,
		const void *data, size_t data_len, uint32_t timeout_ms)
{
	if (!self->enabled) {
		return -ENODEV;
	}

	return i2c_port_write_reg(self, slave_addr, reg_addr, reg_addr_bits,
			data, data_len, timeout_ms);
}
