/*
 * SPDX-FileCopyrightText: 2022 권경환 Kyunghwan Kwon <k@libmcu.org>
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

struct lm_i2c;
struct lm_i2c_device;

struct lm_i2c_bus_api {
	int (*enable)(struct lm_i2c *bus);
	int (*disable)(struct lm_i2c *bus);
	int (*reset)(struct lm_i2c *bus);
	struct lm_i2c_device *(*create_device)(struct lm_i2c *bus,
			uint8_t slave_addr, uint32_t freq_hz);
};

struct lm_i2c_device_api {
	void (*delete_device)(struct lm_i2c_device *dev);

	int (*read)(struct lm_i2c_device *dev,
			void *buf, size_t bufsize, uint32_t timeout_ms);
	int (*write)(struct lm_i2c_device *dev,
			const void *data, size_t data_len, uint32_t timeout_ms);
	int (*read_reg)(struct lm_i2c_device *dev,
			uint32_t reg_addr, uint8_t reg_addr_bits,
			void *buf, size_t bufsize, uint32_t timeout_ms);
	int (*write_reg)(struct lm_i2c_device *dev,
			uint32_t reg_addr, uint8_t reg_addr_bits,
			const void *data, size_t data_len, uint32_t timeout_ms);
};

struct lm_i2c_pin {
	int sda;
	int scl;
};

static inline int lm_i2c_enable(struct lm_i2c *bus) {
	return ((struct lm_i2c_bus_api *)bus)->enable(bus);
}

static inline int lm_i2c_disable(struct lm_i2c *bus) {
	return ((struct lm_i2c_bus_api *)bus)->disable(bus);
}

static inline int lm_i2c_reset(struct lm_i2c *bus) {
	return ((struct lm_i2c_bus_api *)bus)->reset(bus);
}

static inline int lm_i2c_read(struct lm_i2c_device *dev,
		void *buf, size_t bufsize, uint32_t timeout_ms) {
	return ((struct lm_i2c_device_api *)dev)->read(dev,
			buf, bufsize, timeout_ms);
}

static inline int lm_i2c_write(struct lm_i2c_device *dev,
		const void *data, size_t data_len, uint32_t timeout_ms) {
	return ((struct lm_i2c_device_api *)dev)->write(dev,
			data, data_len, timeout_ms);
}

static inline int lm_i2c_read_reg(struct lm_i2c_device *dev,
		uint32_t reg_addr, uint8_t reg_addr_bits,
		void *buf, size_t bufsize, uint32_t timeout_ms) {
	return ((struct lm_i2c_device_api *)dev)->read_reg(dev, reg_addr,
			reg_addr_bits, buf, bufsize, timeout_ms);
}

static inline int lm_i2c_write_reg(struct lm_i2c_device *dev,
		uint32_t reg_addr, uint8_t reg_addr_bits,
		const void *data, size_t data_len, uint32_t timeout_ms) {
	return ((struct lm_i2c_device_api *)dev)->write_reg(dev, reg_addr,
			reg_addr_bits, data, data_len, timeout_ms);
}

static inline struct lm_i2c_device *lm_i2c_create_device(struct lm_i2c *bus,
		uint8_t slave_addr, uint32_t freq_hz) {
	return ((struct lm_i2c_bus_api *)bus)->create_device(bus,
			slave_addr, freq_hz);
}

static inline void lm_i2c_delete_device(struct lm_i2c_device *dev) {
	((struct lm_i2c_device_api *)dev)->delete_device(dev);
}

struct lm_i2c *lm_i2c_create(uint8_t channel, const struct lm_i2c_pin *pin);
void lm_i2c_delete(struct lm_i2c *bus);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_I2C_H */
