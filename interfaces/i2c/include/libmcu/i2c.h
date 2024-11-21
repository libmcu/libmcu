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
struct i2c_device;

struct i2c_bus_api {
	int (*enable)(struct i2c *bus);
	int (*disable)(struct i2c *bus);
	int (*reset)(struct i2c *bus);
	struct i2c_device *(*create_device)(struct i2c *bus,
			uint8_t slave_addr, uint32_t freq_hz);
};

struct i2c_device_api {
	void (*delete_device)(struct i2c_device *device);

	int (*read)(struct i2c_device *dev,
			void *buf, size_t bufsize, uint32_t timeout_ms);
	int (*write)(struct i2c_device *dev,
			const void *data, size_t data_len, uint32_t timeout_ms);
	int (*read_reg)(struct i2c_device *dev,
			uint32_t reg_addr, uint8_t reg_addr_bits,
			void *buf, size_t bufsize, uint32_t timeout_ms);
	int (*write_reg)(struct i2c_device *dev,
			uint32_t reg_addr, uint8_t reg_addr_bits,
			const void *data, size_t data_len, uint32_t timeout_ms);
};

struct i2c_pin {
	int sda;
	int scl;
};

static inline int i2c_enable(struct i2c *bus) {
	return ((struct i2c_bus_api *)bus)->enable(bus);
}

static inline int i2c_disable(struct i2c *bus) {
	return ((struct i2c_bus_api *)bus)->disable(bus);
}

static inline int i2c_reset(struct i2c *bus) {
	return ((struct i2c_bus_api *)bus)->reset(bus);
}

static inline int i2c_read(struct i2c_device *dev,
		void *buf, size_t bufsize, uint32_t timeout_ms) {
	return ((struct i2c_device_api *)dev)->read(dev,
			buf, bufsize, timeout_ms);
}

static inline int i2c_write(struct i2c_device *dev,
		const void *data, size_t data_len, uint32_t timeout_ms) {
	return ((struct i2c_device_api *)dev)->write(dev,
			data, data_len, timeout_ms);
}

static inline int i2c_read_reg(struct i2c_device *dev,
		uint32_t reg_addr, uint8_t reg_addr_bits,
		void *buf, size_t bufsize, uint32_t timeout_ms) {
	return ((struct i2c_device_api *)dev)->read_reg(dev, reg_addr,
			reg_addr_bits, buf, bufsize, timeout_ms);
}

static inline int i2c_write_reg(struct i2c_device *dev,
		uint32_t reg_addr, uint8_t reg_addr_bits,
		const void *data, size_t data_len, uint32_t timeout_ms) {
	return ((struct i2c_device_api *)dev)->write_reg(dev, reg_addr,
			reg_addr_bits, data, data_len, timeout_ms);
}

static inline struct i2c_device *i2c_create_device(struct i2c *bus,
		uint8_t slave_addr, uint32_t freq_hz) {
	return ((struct i2c_bus_api *)bus)->create_device(bus,
			slave_addr, freq_hz);
}

static inline void i2c_delete_device(struct i2c_device *device) {
	((struct i2c_device_api *)device)->delete_device(device);
}

struct i2c *i2c_create(uint8_t channel, const struct i2c_pin *pin);
void i2c_delete(struct i2c *bus);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_I2C_H */
