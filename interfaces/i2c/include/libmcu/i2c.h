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
#include <stdbool.h>

struct i2c;

struct i2c_api {
	int (*enable)(struct i2c *self, uint32_t freq_hz);
	int (*disable)(struct i2c *self);
	int (*read)(struct i2c *self, uint8_t slave_addr,
			void *buf, size_t bufsize, uint32_t timeout_ms);
	int (*write)(struct i2c *self, uint8_t slave_addr,
			const void *data, size_t data_len, uint32_t timeout_ms);
	int (*read_reg)(struct i2c *self, uint8_t slave_addr,
			uint32_t reg_addr, uint8_t reg_addr_bits,
			void *buf, size_t bufsize, uint32_t timeout_ms);
	int (*write_reg)(struct i2c *self, uint8_t slave_addr,
			uint32_t reg_addr, uint8_t reg_addr_bits,
			const void *data, size_t data_len, uint32_t timeout_ms);
};

struct i2c_pin {
	int sda;
	int scl;
};

static inline int i2c_enable(struct i2c *self, uint32_t freq_hz) {
	return ((struct i2c_api *)self)->enable(self, freq_hz);
}

static inline int i2c_disable(struct i2c *self) {
	return ((struct i2c_api *)self)->disable(self);
}

static inline int i2c_read(struct i2c *self, uint8_t slave_addr,
		void *buf, size_t bufsize, uint32_t timeout_ms) {
	return ((struct i2c_api *)self)->read(self,
			slave_addr, buf, bufsize, timeout_ms);
}

static inline int i2c_write(struct i2c *self, uint8_t slave_addr,
		const void *data, size_t data_len, uint32_t timeout_ms) {
	return ((struct i2c_api *)self)->write(self,
			slave_addr, data, data_len, timeout_ms);
}

static inline int i2c_read_reg(struct i2c *self, uint8_t slave_addr,
		uint32_t reg_addr, uint8_t reg_addr_bits,
		void *buf, size_t bufsize, uint32_t timeout_ms) {
	return ((struct i2c_api *)self)->read_reg(self, slave_addr, reg_addr,
			reg_addr_bits, buf, bufsize, timeout_ms);
}

static inline int i2c_write_reg(struct i2c *self, uint8_t slave_addr,
		uint32_t reg_addr, uint8_t reg_addr_bits,
		const void *data, size_t data_len, uint32_t timeout_ms) {
	return ((struct i2c_api *)self)->write_reg(self, slave_addr, reg_addr,
			reg_addr_bits, data, data_len, timeout_ms);
}

struct i2c *i2c_create(uint8_t channel, const struct i2c_pin *pin);
void i2c_delete(struct i2c *self);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_I2C_H */
