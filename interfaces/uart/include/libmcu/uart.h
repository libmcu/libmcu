/*
 * SPDX-FileCopyrightText: 2022 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_UART_H
#define LIBMCU_UART_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#if !defined(LM_GPIO_PIN_UNASSIGNED)
#define LM_GPIO_PIN_UNASSIGNED		-1
#endif

struct lm_uart;

typedef void (*lm_uart_rx_callback_t)(struct lm_uart *self, void *ctx);

typedef enum {
	LM_UART_PARITY_NONE,
	LM_UART_PARITY_EVEN,
	LM_UART_PARITY_ODD,
} lm_uart_parity_t;

typedef enum {
	LM_UART_STOPBIT_1,
	LM_UART_STOPBIT_2,
	LM_UART_STOPBIT_1_5,
} lm_uart_stopbit_t;

typedef enum {
	LM_UART_FLOWCTRL_NONE,
	LM_UART_FLOWCTRL_RTS,
	LM_UART_FLOWCTRL_CTS,
	LM_UART_FLOWCTRL_CTS_RTS,
} lm_uart_flowctrl_t;

struct lm_uart_config {
	uint32_t baudrate;
	uint8_t databit;
	lm_uart_parity_t parity;
	lm_uart_stopbit_t stopbit;
	lm_uart_flowctrl_t flowctrl;
	uint32_t rx_timeout_ms;
};

struct lm_uart_pin {
	int rx;
	int tx;
	int rts;
	int cts;
};

/**
 * @brief Create a UART instance.
 *
 * This function initializes a UART instance with the specified channel and pin
 * configuration.
 *
 * @param[in] channel The UART channel number.
 * @param[in] pin Pointer to the UART pin configuration structure.
 * @return Pointer to the created UART instance, or NULL on failure.
 */
struct lm_uart *lm_uart_create(uint8_t channel, const struct lm_uart_pin *pin);

/**
 * @brief Delete a UART instance.
 *
 * This function deinitializes and frees the resources associated with the
 * specified UART instance.
 *
 * @param[in] self Pointer to the UART instance to be deleted.
 */
void lm_uart_delete(struct lm_uart *self);

/**
 * @brief Enable the UART interface.
 *
 * This function enables the UART interface with the specified baud rate.
 *
 * @param[in] self Pointer to the UART instance.
 * @param[in] baudrate The baud rate for UART communication.
 * @return 0 on success, or a negative error code on failure.
 */
int lm_uart_enable(struct lm_uart *self, uint32_t baudrate);

/**
 * @brief Disable the UART interface.
 *
 * This function disables the UART interface.
 *
 * @param[in] self Pointer to the UART instance.
 * @return 0 on success, or a negative error code on failure.
 */
int lm_uart_disable(struct lm_uart *self);

/**
 * @brief Write data to the UART interface.
 *
 * This function writes the specified data to the UART interface.
 *
 * @param[in] self[] Pointer to the UART instance.
 * @param[in] data Pointer to the data to be written.
 * @param[in] data_len Length of the data to be written.
 * @return Number of bytes written, or a negative error code on failure.
 */
int lm_uart_write(struct lm_uart *self, const void *data, size_t data_len);

/**
 * @brief Read data from the UART interface.
 *
 * This function reads data from the UART interface into the specified buffer.
 *
 * @param[in] self Pointer to the UART instance.
 * @param[out] buf Pointer to the buffer to store the read data.
 * @param[in] bufsize Size of the buffer.
 * @return Number of bytes read, or a negative error code on failure.
 */
int lm_uart_read(struct lm_uart *self, void *buf, size_t bufsize);

/**
 * @brief Register a callback for UART receive events.
 *
 * This function registers a callback function to be called when data is
 * received on the UART interface.
 *
 * @param[in] self Pointer to the UART instance.
 * @param[in] cb Callback function to be called on receive events.
 * @param[in] cb_ctx User-defined context to be passed to the callback function.
 * @return 0 on success, or a negative error code on failure.
 */
int lm_uart_register_rx_callback(struct lm_uart *self,
		lm_uart_rx_callback_t cb, void *cb_ctx);

/**
 * @brief Configure the UART interface.
 *
 * This function configures the UART interface with the specified settings.
 *
 * @param[in] self Pointer to the UART instance.
 * @param[in] config Pointer to the UART configuration structure.
 * @return 0 on success, or a negative error code on failure.
 */
int lm_uart_configure(struct lm_uart *self, const struct lm_uart_config *config);

/**
 * @brief Flush the UART interface.
 *
 * This function flushes the UART interface, clearing any data that may be
 * present in the transmit buffer.
 *
 * @param[in] self Pointer to the UART instance.
 * @return 0 on success, or a negative error code on failure.
 */
int lm_uart_flush(struct lm_uart *self);

/**
 * @brief Clear the UART receive buffer.
 *
 * This function clears the receive buffer of the UART interface, removing any
 * data that may be present in the receive buffer.
 *
 * @param[in] self Pointer to the UART instance.
 * @return 0 on success, or a negative error code on failure.
 */
int lm_uart_clear(struct lm_uart *self);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_UART_H */
