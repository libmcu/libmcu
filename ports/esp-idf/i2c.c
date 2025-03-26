/*
 * SPDX-FileCopyrightText: 2023 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/i2c.h"

#include <errno.h>
#include <string.h>
#include <pthread.h>

#include "driver/i2c_master.h" /* esp-idf */

#if !defined(I2C_MAX_BUS)
#define I2C_MAX_BUS				1
#endif
#if !defined(I2C_MAX_DEVICE)
#define I2C_MAX_DEVICE				8
#endif
#if !defined(I2C_CLK_SRC_DEFAULT)
#define I2C_DEFAULT_TIMEOUT_MS			100
#endif

struct lm_i2c_device {
	struct lm_i2c_device_api api;
	struct lm_i2c *bus;
	uint8_t slave_addr;
	uint32_t freq_hz;
	i2c_master_dev_handle_t handle;
};

struct lm_i2c {
	struct lm_i2c_bus_api api;
	struct lm_i2c_pin pin;
	struct lm_i2c_device devices[I2C_MAX_DEVICE];
	i2c_master_bus_handle_t handle;
	uint8_t channel;
	pthread_mutex_t mutex;
};

static int read_i2c(struct lm_i2c_device *dev,
		void *buf, size_t bufsize, uint32_t timeout_ms)
{
	pthread_mutex_lock(&dev->bus->mutex);
	int err = i2c_master_bus_wait_all_done(dev->bus->handle, timeout_ms);
	/* FIXME: time spent on waiting for the bus is not included in the
	 * timeout. */
	err |= i2c_master_receive(dev->handle, buf, bufsize, timeout_ms);
	pthread_mutex_unlock(&dev->bus->mutex);

	if (err != ESP_OK) {
		return -err;
	}

	return 0;
}

static int write_i2c(struct lm_i2c_device *dev,
		const void *data, size_t data_len, uint32_t timeout_ms)
{
	pthread_mutex_lock(&dev->bus->mutex);
	int err = i2c_master_bus_wait_all_done(dev->bus->handle, timeout_ms);
	err |= i2c_master_transmit(dev->handle, data, data_len, timeout_ms);
	pthread_mutex_unlock(&dev->bus->mutex);

	if (err != ESP_OK) {
		return -err;
	}

	return 0;
}

static int read_reg(struct lm_i2c_device *dev,
		uint32_t reg_addr, uint8_t reg_addr_bits,
		void *buf, size_t bufsize, uint32_t timeout_ms)
{
	size_t reg_len = reg_addr_bits / 8;
	uint8_t reg[4] = {
		(uint8_t)(reg_addr >> 24),
		(uint8_t)(reg_addr >> 16),
		(uint8_t)(reg_addr >> 8),
		(uint8_t)reg_addr
	};

	pthread_mutex_lock(&dev->bus->mutex);
	int err = i2c_master_bus_wait_all_done(dev->bus->handle, timeout_ms);
	err |= i2c_master_transmit_receive(dev->handle, &reg[4 - reg_len],
			reg_len, buf, bufsize, timeout_ms);
	pthread_mutex_unlock(&dev->bus->mutex);

	if (err != ESP_OK) {
		return -err;
	}

	return 0;
}

static int write_reg(struct lm_i2c_device *dev,
		uint32_t reg_addr, uint8_t reg_addr_bits,
		const void *data, size_t data_len, uint32_t timeout_ms)
{
	size_t reg_len = reg_addr_bits / 8;
	size_t len = data_len + reg_len;
	uint8_t *buf = (uint8_t *)malloc(len);
	if (buf == NULL) {
		return -ENOMEM;
	}

	uint8_t reg[4] = {
		(uint8_t)(reg_addr >> 24),
		(uint8_t)(reg_addr >> 16),
		(uint8_t)(reg_addr >> 8),
		(uint8_t)reg_addr
	};

	memcpy(buf, &reg[4 - reg_len], reg_len);

	if (data && data_len) {
		memcpy(&buf[reg_len], data, data_len);
	}

	pthread_mutex_lock(&dev->bus->mutex);
	int err = i2c_master_bus_wait_all_done(dev->bus->handle, timeout_ms);
	err |= i2c_master_transmit(dev->handle, buf, len, timeout_ms);
	pthread_mutex_unlock(&dev->bus->mutex);

	if (err != ESP_OK) {
		return -err;
	}

	free(buf);

	return 0;
}

static struct lm_i2c_device *new_device(struct lm_i2c *bus)
{
	struct lm_i2c_device empty = { 0, };

	for (int i = 0; i < I2C_MAX_DEVICE; i++) {
		struct lm_i2c_device *dev = &bus->devices[i];
		if (memcmp(dev, &empty, sizeof(empty)) == 0) {
			dev->bus = bus;
			dev->api = (struct lm_i2c_device_api) {
				.read = read_i2c,
				.write = write_i2c,
				.read_reg = read_reg,
				.write_reg = write_reg,
			};
			return dev;
		}
	}

	return NULL;
}

static void del_device(struct lm_i2c_device *dev)
{
	memset(dev, 0, sizeof(*dev));
}

static struct lm_i2c_device *create_device(struct lm_i2c *bus,
		uint8_t slave_addr, uint32_t freq_hz)
{
	struct lm_i2c_device *dev = NULL;

	pthread_mutex_lock(&bus->mutex);

	if ((dev = new_device(bus)) == NULL) {
		goto out;
	}

	i2c_device_config_t conf = {
		.scl_speed_hz = freq_hz,
		.device_address = (uint16_t)slave_addr,
	};

	if (i2c_master_bus_add_device(bus->handle, &conf, &dev->handle)
			!= ESP_OK) {
		del_device(dev);
		dev = NULL;
	}

out:
	pthread_mutex_unlock(&bus->mutex);

	return dev;
}

static void delete_device(struct lm_i2c_device *dev)
{
	pthread_mutex_lock(&dev->bus->mutex);
	i2c_master_bus_rm_device(dev->handle);
	del_device(dev);
	pthread_mutex_unlock(&dev->bus->mutex);
}

static int enable_i2c(struct lm_i2c *bus)
{
	i2c_master_bus_config_t conf = {
		.clk_source = I2C_CLK_SRC_DEFAULT, /* SOC_MOD_CLK_APB */
		.i2c_port = bus->channel,
		.scl_io_num = bus->pin.scl,
		.sda_io_num = bus->pin.sda,
		.glitch_ignore_cnt = 7,
		.flags = {
			.enable_internal_pullup = true,
		},
	};

	int err = i2c_new_master_bus(&conf, &bus->handle);

	if (err == ESP_OK) {
		pthread_mutex_init(&bus->mutex, NULL);
		return 0;
	}

	return -err;
}

static int disable_i2c(struct lm_i2c *bus)
{
	pthread_mutex_lock(&bus->mutex);
	int err = i2c_master_bus_wait_all_done(bus->handle,
			I2C_DEFAULT_TIMEOUT_MS);
	err |= i2c_del_master_bus(bus->handle);
	pthread_mutex_unlock(&bus->mutex);
	pthread_mutex_destroy(&bus->mutex);

	return err == ESP_OK? 0 : -err;
}

static int reset_i2c(struct lm_i2c *bus)
{
	pthread_mutex_lock(&bus->mutex);
	int rc = i2c_master_bus_wait_all_done(bus->handle,
			I2C_DEFAULT_TIMEOUT_MS);
	rc |= i2c_master_bus_reset(bus->handle);
	pthread_mutex_unlock(&bus->mutex);
	return rc == ESP_OK? 0 : -rc;
}

struct lm_i2c *lm_i2c_create(uint8_t channel, const struct lm_i2c_pin *pin)
{
	static struct lm_i2c i2c[I2C_MAX_BUS];

	if (channel >= I2C_MAX_BUS) {
		return NULL;
	}

	i2c[channel].api = (struct lm_i2c_bus_api) {
		.enable = enable_i2c,
		.disable = disable_i2c,
		.create_device = create_device,
		.reset = reset_i2c,
	};

	i2c[channel].channel = channel;

	if (pin) {
		memcpy(&i2c[channel].pin, pin, sizeof(*pin));
	}

	return &i2c[channel];
}

void lm_i2c_delete(struct lm_i2c *bus)
{
	memset(bus, 0, sizeof(*bus));
}
