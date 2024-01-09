/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_SPI_H
#define LIBMCU_SPI_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

struct spi;

struct spi_api {
	int (*enable)(struct spi *self);
	int (*disable)(struct spi *self);
	int (*write)(struct spi *self, const void *data, size_t data_len);
	int (*read)(struct spi *self, void *buf, size_t bufsize);
	int (*writeread)(struct spi *self, const void *txdata,
			size_t txdata_len, void *rxbuf, size_t rxbuf_len);
};

static inline int spi_enable(struct spi *self) {
	return ((struct spi_api *)self)->enable(self);
}

static inline int spi_disable(struct spi *self) {
	return ((struct spi_api *)self)->disable(self);
}

static inline int spi_write(struct spi *self,
		const void *data, size_t data_len) {
	return ((struct spi_api *)self)->write(self, data, data_len);
}

static inline int spi_read(struct spi *self, void *buf, size_t bufsize) {
	return ((struct spi_api *)self)->read(self, buf, bufsize);
}

static inline int spi_writeread(struct spi *self, const void *txdata,
		size_t txdata_len, void *rxbuf, size_t rxbuf_len) {
	return ((struct spi_api *)self)->writeread(self,
			txdata, txdata_len, rxbuf, rxbuf_len);
}

struct spi *spi_create(uint8_t channel);
void spi_delete(struct spi *self);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_SPI_H */
