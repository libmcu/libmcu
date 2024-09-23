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

typedef enum {
	SPI_MODE_0, /* CPOL=0, CPHA=0 */
	SPI_MODE_1, /* CPOL=0, CPHA=1 */
	SPI_MODE_2, /* CPOL=1, CPHA=0 */
	SPI_MODE_3, /* CPOL=1, CPHA=1 */
} spi_mode_t;

struct spi;

struct spi_api {
	int (*enable)(struct spi *self, spi_mode_t mode, uint32_t freq_hz);
	int (*disable)(struct spi *self);
	int (*write)(struct spi *self, const void *data, size_t data_len);
	int (*read)(struct spi *self, void *buf, size_t bufsize);
	int (*writeread)(struct spi *self, const void *txdata,
			size_t txdata_len, void *rxbuf, size_t rxbuf_len);
};

struct spi_pin {
	int miso;
	int mosi;
	int sclk;
	int default_cs;
};

static inline int spi_enable(struct spi *self,
		spi_mode_t mode, uint32_t freq_hz) {
	return ((struct spi_api *)self)->enable(self, mode, freq_hz);
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

struct spi *spi_create(uint8_t channel, const struct spi_pin *pin);
void spi_delete(struct spi *self);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_SPI_H */
