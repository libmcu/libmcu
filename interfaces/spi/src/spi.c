/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/spi.h"
#include "libmcu/port/spi.h"

struct spi *spi_create(uint8_t channel)
{
	return spi_port_create(channel);
}

void spi_delete(struct spi *self)
{
	spi_port_delete(self);
}

int spi_enable(struct spi *self)
{
	return spi_port_enable(self);
}

int spi_disable(struct spi *self)
{
	return spi_port_disable(self);
}

int spi_write(struct spi *self, const void *data, size_t data_len)
{
	return spi_port_write(self, data, data_len);
}

int spi_read(struct spi *self, void *buf, size_t bufsize)
{
	return spi_port_read(self, buf, bufsize);
}

int spi_writeread(struct spi *self, const void *txdata, size_t txdata_len,
		void *rxbuf, size_t rxbuf_len)
{
	return spi_port_writeread(self, txdata, txdata_len, rxbuf, rxbuf_len);
}
