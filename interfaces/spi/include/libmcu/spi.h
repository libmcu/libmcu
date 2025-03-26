/*
 * SPDX-FileCopyrightText: 2022 권경환 Kyunghwan Kwon <k@libmcu.org>
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

#if !defined(LM_SPI_PIN_UNASSIGNED)
#define LM_SPI_PIN_UNASSIGNED		-1
#endif

typedef enum {
	LM_SPI_MODE_0, /* CPOL=0, CPHA=0 */
	LM_SPI_MODE_1, /* CPOL=0, CPHA=1 */
	LM_SPI_MODE_2, /* CPOL=1, CPHA=0 */
	LM_SPI_MODE_3, /* CPOL=1, CPHA=1 */
} lm_spi_mode_t;

struct lm_spi_pin {
	int miso;
	int mosi;
	int sclk;
};

struct lm_spi;
struct lm_spi_device;

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
struct lm_spi *lm_spi_create(uint8_t channel, const struct lm_spi_pin *pin);

/**
 * @brief Delete a SPI instance.
 *
 * This function deletes a SPI instance and frees the associated resources.
 *
 * @param[in] self The SPI instance to be deleted.
 */
void lm_spi_destroy(struct lm_spi *self);

/**
 * Creates a new SPI device.
 *
 * @param[in] self A pointer to the SPI structure.
 * @param[in] mode The SPI mode.
 * @param[in] freq_hz The frequency in Hz.
 * @param[in] pin_cs The chip select pin.
 *
 * @return A pointer to the created SPI device.
 */
struct lm_spi_device *lm_spi_create_device(struct lm_spi *self,
		lm_spi_mode_t mode, uint32_t freq_hz, int pin_cs);

/**
 * Deletes an SPI device.
 *
 * @param[in] dev A pointer to the SPI device to delete.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int lm_spi_delete_device(struct lm_spi_device *dev);

/**
 * Enables an SPI device.
 *
 * @param[in] dev A pointer to the SPI device to enable.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int lm_spi_enable(struct lm_spi_device *dev);

/**
 * Disables an SPI device.
 *
 * @param[in] dev A pointer to the SPI device to disable.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int lm_spi_disable(struct lm_spi_device *dev);

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
int lm_spi_write(struct lm_spi_device *dev, const void *data, size_t data_len);

/**
 * @brief Read data from a SPI device.
 *
 * This function reads data from a SPI device into a buffer.
 *
 * @param[in] dev The SPI device.
 * @param[out] buf The buffer where the read data will be stored.
 * @param[in] rx_len The length of the data to be read.
 *
 * @return 0 if the operation is successful, otherwise returns a non-zero error
 *         code.
 */
int lm_spi_read(struct lm_spi_device *dev, void *buf, size_t rx_len);

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
 * @param[in] rx_len The length of the data to be read.
 *
 * @return 0 if the operation is successful, otherwise returns a non-zero error
 *         code.
 */
int lm_spi_writeread(struct lm_spi_device *dev,
		const void *txdata, size_t txdata_len,
		void *rxbuf, size_t rx_len);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_SPI_H */
