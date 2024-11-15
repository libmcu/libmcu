/*
 * SPDX-FileCopyrightText: 2018 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_XMODEM_H
#define LIBMCU_XMODEM_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

typedef enum {
	XMODEM_ERROR_NONE,
	XMODEM_ERROR,
	XMODEM_ERROR_INVALID_PARAM,
	XMODEM_ERROR_NO_ENOUGH_BUFFER,
	XMODEM_ERROR_TIMEOUT,
	XMODEM_ERROR_NO_RESPONSE,
	XMODEM_ERROR_NO_INPUT,
	XMODEM_ERROR_CANCELED,
	XMODEM_ERROR_CANCELED_BY_REMOTE,
	XMODEM_ERROR_CRC_MISMATCH,
	XMODEM_ERROR_INCORRECT_SEQ,
	XMODEM_ERROR_RETRY,
	XMODEM_ERROR_RECEIVING,
} xmodem_error_t;

typedef enum {
	XMODEM_DATA_BLOCK_128, /* NAK. DLE '6' C' */
	XMODEM_DATA_BLOCK_128_CRC,
	XMODEM_DATA_BLOCK_1K,  /* 'C'. DLE '4' C' */
} xmodem_data_block_size_t;

/**
 * @brief Callback function for receiving XMODEM packets.
 *
 * The callback function that will be called when an XMODEM packet is received.
 *
 * @note seq will be 0 for the last packet of EOT.
 *
 * @param[in] seq The sequence number of the received packet.
 * @param[in] data The pointer to the data of the received packet.
 * @param[in] datasize The size of the received data.
 * @param[in] ctx The context passed to the callback function.
 *
 * @return The error status of the receive operation.
 */
typedef xmodem_error_t (*xmodem_recv_callback_t)(size_t seq,
		const uint8_t *data, size_t datasize, void *ctx);

typedef int (*xmodem_reader_t)(uint8_t *buf, size_t bufsize,
		uint32_t timeout_ms);
typedef int (*xmodem_writer_t)(const uint8_t *data, size_t datasize,
		uint32_t timeout_ms);
typedef void (*xmodem_flush_t)(void);

typedef uint32_t (*xmodem_millis_t)(void);

/**
 * @brief Receives data using the XMODEM protocol.
 *
 * This function receives data packets using the XMODEM protocol with the
 * specified block size and timeout. It also calls a callback function when a
 * packet is received.
 *
 * @param[in] block_type The size of the data block to be received.
 * @param[in] on_recv The callback function to be called when a packet is
 *                    received.
 * @param[in] on_recv_ctx The context to be passed to the callback function.
 * @param[in] timeout_ms The timeout in milliseconds for the receive operation.
 *
 * @return The error status of the receive operation.
 */
xmodem_error_t xmodem_receive(xmodem_data_block_size_t block_type,
		xmodem_recv_callback_t on_recv, void *on_recv_ctx,
		uint32_t timeout_ms);

/**
 * @brief Set the I/O functions for XMODEM communication.
 *
 * This function sets the reader, writer, and flush functions to be used for
 * XMODEM communication. These functions are used to read from, write to, and
 * flush the communication interface, respectively.
 *
 * @param[in] reader The function to read data from the communication interface.
 * @param[in] writer The function to write data to the communication interface.
 * @param[in] flush The function to flush the communication interface.
 */
void xmodem_set_io(xmodem_reader_t reader, xmodem_writer_t writer,
		xmodem_flush_t flush);

/**
 * @brief Sets the receive buffer for the XMODEM protocol.
 *
 * This function sets the buffer that will be used to store incoming data
 * packets received via the XMODEM protocol.
 *
 * @note The buffer must be set before calling xmodem_receive(). The buffer must
 * be large enough to store the incoming data packets. At least 128 bytes for
 * 128-byte data or 1024 bytes for 1KB data should be provided. The buffer is
 * the one passed to the callback function.
 *
 * @param[out] rx_buf The buffer to store received data.
 * @param[in] rx_bufsize The size of the receive buffer.
 */
void xmodem_set_rx_buffer(uint8_t *rx_buf, size_t rx_bufsize);

/**
 * @brief Sets the function to get the current time in milliseconds.
 *
 * This function sets the function pointer that will be used to get the
 * current time in milliseconds. This is used by the XMODEM protocol for
 * timing operations.
 *
 * @param[in] millis The function pointer to the function that returns the current
 *                   time in milliseconds.
 */
void xmodem_set_millis(xmodem_millis_t fn);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_XMODEM_H */
