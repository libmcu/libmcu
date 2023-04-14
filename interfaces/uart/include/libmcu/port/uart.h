/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_UART_PORT_H
#define LIBMCU_UART_PORT_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "libmcu/uart.h"

struct uart *uart_port_create(uint8_t channel);
void uart_port_delete(struct uart *self);
int uart_port_enable(struct uart *self, uint32_t baudrate);
int uart_port_disable(struct uart *self);
int uart_port_write(struct uart *self, const void *data, size_t data_len);
int uart_port_read(struct uart *self, void *buf, size_t bufsize);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_UART_PORT_H */
