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

struct spi_pin {
	int miso;
	int mosi;
	int sclk;
};

struct spi;
struct spi_device;

/**
 * @brief Create a SPI instance.
 *
 * This function creates a SPI instance associated with the given channel and
 * pin.
 *
 * @param[in] channel The channel to be associated with the SPI instance.
 * @param[in] pin The pin to be associated with the SPI instance.
 *
 * @return A pointer to the created SPI instance. If the creation fails,
 *         the function returns NULL.
 */
struct spi *spi_create(uint8_t channel, const struct spi_pin *pin);

/**
 * @brief Delete a SPI instance.
 *
 * This function deletes a SPI instance and frees the associated resources.
 *
 * @param[in] self The SPI instance to be deleted.
 */
void spi_delete(struct spi *self);

/**
 * @brief Enable a SPI device.
 *
 * This function enables a SPI device with a given mode, frequency, and chip
 * select pin.
 *
 * @param[in] self The SPI instance.
 * @param[in] mode The SPI mode.
 * @param[in] freq_hz The frequency of the SPI signal in Hz.
 * @param[in] pin_cs The chip select pin for the SPI device.
 *
 * @return A pointer to the enabled SPI device. If the operation fails,
 *         the function returns NULL.
 */
struct spi_device *spi_enable(struct spi *self,
		spi_mode_t mode, uint32_t freq_hz, int pin_cs);

/**
 * @brief Disable a SPI device.
 *
 * This function disables a SPI device and frees the associated resources.
 *
 * @param[in] dev The SPI device to be disabled.
 *
 * @return 0 if the operation is successful, otherwise returns a non-zero error
 *         code.
 */
int spi_disable(struct spi_device *dev);

/**
 * @brief Write data to a SPI device.
 *
 * This function writes data to a SPI device.
 *
 * @param[in] dev The SPI device.
 * @param[in] data The data to be written.
 * @param[in] data_len The length of the data to be written.
 *
 * @return 0 if the operation is successful, otherwise returns a non-zero error
 *         code.
 */
int spi_write(struct spi_device *dev, const void *data, size_t data_len);

/**
 * @brief Read data from a SPI device.
 *
 * This function reads data from a SPI device into a buffer.
 *
 * @param[in] dev The SPI device.
 * @param[out] buf The buffer where the read data will be stored.
 * @param[in] bufsize The size of the buffer.
 *
 * @return 0 if the operation is successful, otherwise returns a non-zero error
 *         code.
 */
int spi_read(struct spi_device *dev, void *buf, size_t bufsize);

/**
 * @brief Write and read data from a SPI device.
 *
 * This function writes data to a SPI device and then reads data from the
 * device.
 *
 * @param[in] dev The SPI device.
 * @param[in] txdata The data to be written.
 * @param[in] txdata_len The length of the data to be written.
 * @param[out] rxbuf The buffer where the read data will be stored.
 * @param[in] rxbuf_len The size of the buffer.
 *
 * @return 0 if the operation is successful, otherwise returns a non-zero error
 *         code.
 */
int spi_writeread(struct spi_device *dev,
		const void *txdata, size_t txdata_len,
		void *rxbuf, size_t rxbuf_len);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_SPI_H */
