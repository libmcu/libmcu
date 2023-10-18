/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_SPI_PORT_H
#define LIBMCU_SPI_PORT_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "libmcu/spi.h"

struct spi *spi_port_create(uint8_t channel);
void spi_port_delete(struct spi *self);
int spi_port_enable(struct spi *self);
int spi_port_disable(struct spi *self);
int spi_port_write(struct spi *self, const void *data, size_t data_len);
int spi_port_read(struct spi *self, void *buf, size_t bufsize);
int spi_port_writeread(struct spi *self, const void *txdata, size_t txdata_len,
		void *rxbuf, size_t rxbuf_len);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_SPI_PORT_H */
