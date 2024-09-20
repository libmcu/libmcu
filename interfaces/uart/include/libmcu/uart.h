/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@libmcu.org>
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

struct uart;

typedef void (*uart_rx_callback_t)(struct uart *self, void *ctx);

struct uart_api {
	int (*enable)(struct uart *self, uint32_t baudrate);
	int (*disable)(struct uart *self);
	int (*write)(struct uart *self, const void *data, size_t data_len);
	int (*read)(struct uart *self, void *buf, size_t bufsize);
	int (*register_rx_callback)(struct uart *self,
			uart_rx_callback_t cb, void *cb_ctx);
};

struct uart_pin {
	int rx;
	int tx;
	int rts;
	int cts;
};

static inline int uart_enable(struct uart *self, uint32_t baudrate) {
	return ((struct uart_api *)self)->enable(self, baudrate);
}

static inline int uart_disable(struct uart *self) {
	return ((struct uart_api *)self)->disable(self);
}

static inline int uart_write(struct uart *self,
		const void *data, size_t data_len) {
	return ((struct uart_api *)self)->write(self, data, data_len);
}

static inline int uart_read(struct uart *self, void *buf, size_t bufsize) {
	return ((struct uart_api *)self)->read(self, buf, bufsize);
}

static inline int uart_register_rx_callback(struct uart *self,
		uart_rx_callback_t cb, void *cb_ctx) {
	return ((struct uart_api *)self)->register_rx_callback(self,
			cb, cb_ctx);
}

struct uart *uart_create(uint8_t channel, const struct uart_pin *pin);
void uart_delete(struct uart *self);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_UART_H */
