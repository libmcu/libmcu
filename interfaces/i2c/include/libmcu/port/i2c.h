/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_I2C_PORT_H
#define LIBMCU_I2C_PORT_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "libmcu/i2c.h"

int i2c_port_init(struct i2c *self);
int i2c_port_deinit(struct i2c *self);
int i2c_port_write(struct i2c *self, uint8_t addr, uint8_t reg,
		const void *data, size_t data_len);
int i2c_port_read(struct i2c *self, uint8_t addr, uint8_t reg,
		void *buf, size_t bufsize);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_I2C_PORT_H */
