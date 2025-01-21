/*
 * SPDX-FileCopyrightText: 2023 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/spi.h"
#include "libmcu/compiler.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "driver/spi_master.h"

#if !defined(SPI_MAX_DEVICES)
#define SPI_MAX_DEVICES		4
#endif

#define MAX_SPI			2 /* SPI2 and SPI3 */
#define DEFAULT_QUEUE_SIZE	7

struct spi_device {
	struct spi *spi;
	spi_mode_t mode;
	uint32_t freq_hz;
	int pin_cs;

	spi_device_handle_t handle;

	bool enabled;
};

struct spi {
	struct spi_pin pin;

	spi_host_device_t host;

	struct spi_device device[SPI_MAX_DEVICES];
};

static bool is_bus_initialized(const struct spi *self)
{
	for (int i = 0; i < SPI_MAX_DEVICES; i++) {
		if (self->device[i].handle) {
			return true;
		}
	}

	return false;
}

static bool is_already_assigned_pin(const struct spi *self, int pin_cs)
{
	for (int i = 0; i < SPI_MAX_DEVICES; i++) {
		if (self->device[i].pin_cs == pin_cs) {
			return true;
		}
	}

	return false;
}

static struct spi_device *alloc_device(struct spi *self)
{
	for (int i = 0; i < SPI_MAX_DEVICES; i++) {
		if (self->device[i].handle == NULL) {
			return &self->device[i];
		}
	}

	return NULL;
}

static void free_device(struct spi_device *device)
{
	device->spi = NULL;
	device->pin_cs = SPI_PIN_UNASSIGNED;
	device->handle = NULL;
}

static int count_assigned_devices(struct spi *self)
{
	int count = 0;

	for (int i = 0; i < SPI_MAX_DEVICES; i++) {
		if (self->device[i].handle) {
			count++;
		}
	}

	return count;
}

static int convert_freq_to_clock_div(uint32_t freq_hz)
{
	if (freq_hz >= 80000000) {
		return SPI_MASTER_FREQ_80M;
	} else if (freq_hz >= 40000000) {
		return SPI_MASTER_FREQ_40M;
	} else if (freq_hz >= 26000000) {
		return SPI_MASTER_FREQ_26M;
	} else if (freq_hz >= 20000000) {
		return SPI_MASTER_FREQ_20M;
	} else if (freq_hz >= 16000000) {
		return SPI_MASTER_FREQ_16M;
	} else if (freq_hz >= 13000000) {
		return SPI_MASTER_FREQ_13M;
	} else if (freq_hz >= 11000000) {
		return SPI_MASTER_FREQ_11M;
	} else if (freq_hz >= 10000000) {
		return SPI_MASTER_FREQ_10M;
	} else if (freq_hz >= 9000000){
		return SPI_MASTER_FREQ_9M;
	}

	return SPI_MASTER_FREQ_8M;
}

static int disable_spi_device(struct spi_device *dev, bool free_after_disable)
{
	if (!dev->enabled || !dev->handle) {
		return -EALREADY;
	}

	esp_err_t err = spi_bus_remove_device(dev->handle);

	if (err != ESP_OK) {
		return err;
	}

	dev->enabled = false;

	if (free_after_disable) {
		free_device(dev);
	}

	if (count_assigned_devices(dev->spi) == 0) {
		return err | spi_bus_free(dev->spi->host);
	}

	return 0;
}

int spi_write(struct spi_device *dev, const void *data, size_t data_len)
{
	if (!data || data_len == 0) {
		return -EINVAL;
	}

	spi_transaction_t t = {
		.length = data_len * 8,
		.tx_buffer = data,
		.user = (void *)dev,
	};

	if (spi_device_polling_transmit(dev->handle, &t) != ESP_OK) {
		return -EIO;
	}

	return (int)data_len;
}

int spi_writeread(struct spi_device *dev,
		const void *txdata, size_t txdata_len,
		void *rxbuf, size_t rxbuf_len)
{
	int rc = 0;
	spi_transaction_t transaction = {
		.length = txdata_len * 8,
		.rxlength = rxbuf_len * 8,
		.tx_buffer = txdata,
		.rx_buffer = rxbuf,
		.user = (void *)dev,
	};

	if (spi_device_transmit(dev->handle, &transaction) != ESP_OK) {
		rc = -EIO;
	}

	return rc;
}

int spi_read(struct spi_device *dev, void *buf, size_t bufsize)
{
	unused(dev);
	unused(buf);
	unused(bufsize);
	return -ENOTSUP;
}

int spi_enable(struct spi_device *dev)
{
	if (!is_bus_initialized(dev->spi)) {
		spi_bus_config_t buscfg = {
			.miso_io_num = dev->spi->pin.miso,
			.mosi_io_num = dev->spi->pin.mosi,
			.sclk_io_num = dev->spi->pin.sclk,
			.quadwp_io_num = -1,
			.quadhd_io_num = -1,
			.flags = SPICOMMON_BUSFLAG_MASTER,
		};
		esp_err_t err = spi_bus_initialize(dev->spi->host,
				     &buscfg, SPI_DMA_CH_AUTO);
		if (err != ESP_OK) {
			return err;
		}
	}

	spi_device_interface_config_t devcfg = {
		.clock_speed_hz = convert_freq_to_clock_div(dev->freq_hz),
		.mode = dev->mode,
		.spics_io_num = dev->pin_cs,
		.queue_size = DEFAULT_QUEUE_SIZE,
		/*.flags = SPI_DEVICE_HALFDUPLEX,*/
	};

	esp_err_t err =
		spi_bus_add_device(dev->spi->host, &devcfg, &dev->handle);

	if (err == ESP_OK) {
		dev->enabled = true;
	}

	return err;
}

int spi_disable(struct spi_device *dev)
{
	return disable_spi_device(dev, false);
}

struct spi_device *spi_create_device(struct spi *self,
		spi_mode_t mode, uint32_t freq_hz, int pin_cs)
{
	struct spi_device *device = alloc_device(self);

	if (device) {
		device->spi = self;
		device->mode = mode;
		device->freq_hz = freq_hz;
		device->pin_cs = pin_cs;
	}

	return device;
}

int spi_delete_device(struct spi_device *dev)
{
	return disable_spi_device(dev, true);
}

struct spi *spi_create(uint8_t channel, const struct spi_pin *pin)
{
	static struct spi spi[MAX_SPI];

	/* accept only SPI2 and SPI3 */
	if (channel < 2 || channel >= (2 + MAX_SPI)) {
		return NULL;
	}

	channel -= 2;

	switch (channel) {
	case 0:
#if defined(esp32)
		spi[channel].host = HSPI_HOST;
#else
		spi[channel].host = SPI2_HOST;
#endif
		break;
	case 1:
#if defined(esp32)
		spi[channel].host = VSPI_HOST;
#else
		spi[channel].host = SPI3_HOST;
#endif
		break;
	}

	if (pin) {
		memcpy(&spi[channel].pin, pin, sizeof(*pin));
	}

	for (int i = 0; i < SPI_MAX_DEVICES; i++) {
		free_device(&spi[channel].device[i]);
	}

	return &spi[channel];
}

void spi_delete(struct spi *self)
{
	for (int i = 0; i < SPI_MAX_DEVICES; i++) {
		spi_delete_device(&self->device[i]);
	}
}
