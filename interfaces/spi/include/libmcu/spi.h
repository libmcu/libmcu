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

struct spi *spi_create(uint8_t channel);
void spi_delete(struct spi *self);
int spi_enable(struct spi *self);
int spi_disable(struct spi *self);
int spi_write(struct spi *self, const void *data, size_t data_len);
int spi_read(struct spi *self, void *buf, size_t bufsize);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_SPI_H */
