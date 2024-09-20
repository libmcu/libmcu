/*
 * SPDX-FileCopyrightText: 2023 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/i2c.h"

#include <errno.h>
#include <string.h>
#include <pthread.h>

#include "driver/i2c.h" /* esp-idf */

#define MAX_I2C				1

struct i2c {
	struct i2c_api api;
	struct i2c_pin pin;
	uint8_t channel;
	pthread_mutex_t mutex;
};

static int read_i2c(struct i2c *self, uint8_t slave_addr,
		void *buf, size_t bufsize, uint32_t timeout_ms)
{
	pthread_mutex_lock(&self->mutex);
	int rc = i2c_master_read_from_device(self->channel, slave_addr,
			buf, bufsize, pdMS_TO_TICKS(timeout_ms));
	pthread_mutex_unlock(&self->mutex);

	if (rc != ESP_OK) {
		return -EIO;
	}

	return 0;
}

static int write_i2c(struct i2c *self, uint8_t slave_addr,
		const void *data, size_t data_len, uint32_t timeout_ms)
{
	pthread_mutex_lock(&self->mutex);
	int rc = i2c_master_write_to_device(self->channel, slave_addr,
			data, data_len, pdMS_TO_TICKS(timeout_ms));
	pthread_mutex_unlock(&self->mutex);

	if (rc != ESP_OK) {
		rc = -EIO;
	}

	return rc;
}

static int read_reg(struct i2c *self, uint8_t slave_addr,
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

	pthread_mutex_lock(&self->mutex);
	int rc = i2c_master_write_read_device(self->channel, slave_addr,
			&reg[4 - reg_len], reg_len, buf, bufsize,
			pdMS_TO_TICKS(timeout_ms));
	pthread_mutex_unlock(&self->mutex);

	if (rc != ESP_OK) {
		return -EIO;
	}

	return 0;
}

static int write_reg(struct i2c *self, uint8_t slave_addr,
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

	pthread_mutex_lock(&self->mutex);
	int rc = i2c_master_write_to_device(self->channel, slave_addr,
			buf, len, pdMS_TO_TICKS(timeout_ms));
	pthread_mutex_unlock(&self->mutex);

	if (rc != ESP_OK) {
		rc = -EIO;
	}

	free(buf);

	return rc;
}

static int enable_i2c(struct i2c *self, uint32_t freq_hz)
{
	pthread_mutex_init(&self->mutex, NULL);

	i2c_config_t conf = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = self->pin.sda,
		.scl_io_num = self->pin.scl,
		.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.scl_pullup_en = GPIO_PULLUP_ENABLE,
		.master.clk_speed = freq_hz,
	};

	if (i2c_param_config(self->channel, &conf) != ESP_OK) {
		return -EINVAL;
	}

	if (i2c_driver_install(self->channel, conf.mode, 0, 0, 0) != ESP_OK) {
		return -EFAULT;
	}

	return 0;
}

static int disable_i2c(struct i2c *self)
{
	pthread_mutex_destroy(&self->mutex);

	if (i2c_driver_delete(self->channel) != ESP_OK) {
		return -EFAULT;
	}

	return 0;
}

struct i2c *i2c_create(uint8_t channel, const struct i2c_pin *pin)
{
	static struct i2c i2c[MAX_I2C];

	if (channel >= MAX_I2C) {
		return NULL;
	}

	i2c[channel].api = (struct i2c_api) {
		.enable = enable_i2c,
		.disable = disable_i2c,
		.read = read_i2c,
		.write = write_i2c,
		.read_reg = read_reg,
		.write_reg = write_reg,
	};

	i2c[channel].channel = channel;

	if (pin) {
		memcpy(&i2c[channel].pin, pin, sizeof(*pin));
	}

	return &i2c[channel];
}

void i2c_delete(struct i2c *self)
{
	memset(self, 0, sizeof(*self));
}
