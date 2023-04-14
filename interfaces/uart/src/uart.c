/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/uart.h"
#include "libmcu/port/uart.h"

struct uart *uart_create(uint8_t channel)
{
	return uart_port_create(channel);
}

void uart_delete(struct uart *self)
{
	uart_port_delete(self);
}

int uart_enable(struct uart *self, uint32_t baudrate)
{
	return uart_port_enable(self, baudrate);
}

int uart_disable(struct uart *self)
{
	return uart_port_disable(self);
}

int uart_write(struct uart *self, const void *data, size_t data_len)
{
	return uart_port_write(self, data, data_len);
}

int uart_read(struct uart *self, void *buf, size_t bufsize)
{
	return uart_port_read(self, buf, bufsize);
}
