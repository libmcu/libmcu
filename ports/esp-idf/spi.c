/*
 * SPDX-FileCopyrightText: 2023 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/spi.h"
#include "libmcu/compiler.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "driver/spi_master.h"

#define MAX_SPI			2

#if defined(esp32)
#define SPI_HOST		HSPI_HOST
#else
#define SPI_HOST		SPI2_HOST
#endif

#define DEFAULT_QUEUE_SIZE	7

struct spi {
	struct spi_api api;
	struct spi_pin pin;
	spi_device_handle_t handle;
};

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
		return SPI_MASTER_FREQ_13M;
	} else if (freq_hz >= 13000000) {
		return SPI_MASTER_FREQ_11M;
	} else if (freq_hz >= 11000000) {
		return SPI_MASTER_FREQ_10M;
	} else if (freq_hz >= 10000000) {
		return SPI_MASTER_FREQ_9M;
	}

	return SPI_MASTER_FREQ_8M;
}

static int write_spi(struct spi *self, const void *data, size_t data_len)
{
	if (!data || data_len == 0) {
		return -EINVAL;
	}

	spi_transaction_t t = {
		.length = data_len * 8,
		.tx_buffer = data,
		.user = (void *)self,
	};

	if (spi_device_polling_transmit(self->handle, &t) != ESP_OK) {
		return -EIO;
	}

	return (int)data_len;
}

static int writeread(struct spi *self, const void *txdata, size_t txdata_len,
		       void *rxbuf, size_t rxbuf_len)
{
	int rc = 0;
	uint8_t *buf = (uint8_t *)calloc(1, txdata_len + rxbuf_len);

	if (buf == NULL) {
		return -ENOMEM;
	}

	memcpy(buf, txdata, txdata_len);

	spi_transaction_t transaction = {
		.length = txdata_len * 8,
		.rxlength = rxbuf_len * 8,
		.tx_buffer = buf,
		.rx_buffer = &buf[txdata_len],
		.user = (void *)self,
	};

	if (spi_device_transmit(self->handle, &transaction) != ESP_OK) {
		rc = -EIO;
	}

	memcpy(rxbuf, &buf[txdata_len], rxbuf_len);

	free(buf);

	return rc;
}

static int read_spi(struct spi *self, void *buf, size_t bufsize)
{
	unused(self);
	unused(buf);
	unused(bufsize);
	return -ENOTSUP;
}

static int enable_spi(struct spi *self, spi_mode_t mode, uint32_t freq_hz)
{
	spi_bus_config_t buscfg = {
		.miso_io_num = self->pin.miso,
		.mosi_io_num = self->pin.mosi,
		.sclk_io_num = self->pin.sclk,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
		/*.flags = SPICOMMON_BUSFLAG_MASTER,*/
	};
	esp_err_t ret = spi_bus_initialize(SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
	ESP_ERROR_CHECK(ret);

	spi_device_interface_config_t devcfg = {
		.clock_speed_hz = convert_freq_to_clock_div(freq_hz),
		.mode = mode,
		.spics_io_num = -1,
		.queue_size = DEFAULT_QUEUE_SIZE,
		/*.flags = SPI_DEVICE_HALFDUPLEX,*/
	};

	ret = spi_bus_add_device(SPI_HOST, &devcfg, &self->handle);
	ESP_ERROR_CHECK(ret);

	return 0;
}

static int disable_spi(struct spi *self)
{
	spi_bus_remove_device(self->handle);
	spi_bus_free(SPI_HOST);
	return 0;
}

struct spi *spi_create(uint8_t channel, const struct spi_pin *pin)
{
	static struct spi spi[MAX_SPI];

	if (channel == 0 || channel > MAX_SPI) {
		return NULL;
	}

	spi[channel].api = (struct spi_api) {
		.enable = enable_spi,
		.disable = disable_spi,
		.write = write_spi,
		.read = read_spi,
		.writeread = writeread,
	};

	if (pin) {
		memcpy(&spi[channel].pin, pin, sizeof(*pin));
	}

	return &spi[channel];
}

void spi_delete(struct spi *self)
{
	unused(self);
}
