/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_I2C_H
#define LIBMCU_I2C_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

struct i2c;

struct i2c *i2c_create(uint8_t id);
void i2c_delete(struct i2c *self);
int i2c_write(struct i2c *self, uint8_t addr, uint8_t reg,
		const void *data, size_t data_len);
int i2c_read(struct i2c *self, uint8_t addr, uint8_t reg,
		void *buf, size_t bufsize);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_I2C_H */
