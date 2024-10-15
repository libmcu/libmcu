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

#if !defined(SPI_PIN_UNASSIGNED)
#define SPI_PIN_UNASSIGNED		-1
#endif

typedef enum {
	SPI_MODE_0, /* CPOL=0, CPHA=0 */
	SPI_MODE_1, /* CPOL=0, CPHA=1 */
	SPI_MODE_2, /* CPOL=1, CPHA=0 */
	SPI_MODE_3, /* CPOL=1, CPHA=1 */
} spi_mode_t;

struct spi;
struct spi_device;

struct spi_api {
	struct spi_device *(*enable)(struct spi *self,
			spi_mode_t mode, uint32_t freq_hz, int pin_cs);
	int (*disable)(struct spi_device *dev);
	int (*write)(struct spi_device *dev,
			const void *data, size_t data_len);
	int (*read)(struct spi_device *dev, void *buf, size_t bufsize);
	int (*writeread)(struct spi_device *dev, const void *txdata,
			size_t txdata_len, void *rxbuf, size_t rxbuf_len);
};

struct spi_pin {
	int miso;
	int mosi;
	int sclk;
};

static inline struct spi_device *spi_enable(struct spi *self,
		spi_mode_t mode, uint32_t freq_hz, int pin_cs) {
	return ((struct spi_api *)self)->enable(self, mode, freq_hz, pin_cs);
}

static inline int spi_disable(struct spi_device *dev) {
	return ((struct spi_api *)dev)->disable(dev);
}

static inline int spi_write(struct spi_device *dev,
		const void *data, size_t data_len) {
	return ((struct spi_api *)dev)->write(dev, data, data_len);
}

static inline int spi_read(struct spi_device *dev, void *buf, size_t bufsize) {
	return ((struct spi_api *)dev)->read(dev, buf, bufsize);
}

static inline int spi_writeread(struct spi_device *dev, const void *txdata,
		size_t txdata_len, void *rxbuf, size_t rxbuf_len) {
	return ((struct spi_api *)dev)->writeread(dev,
			txdata, txdata_len, rxbuf, rxbuf_len);
}

struct spi *spi_create(uint8_t channel, const struct spi_pin *pin);
void spi_delete(struct spi *self);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_SPI_H */
