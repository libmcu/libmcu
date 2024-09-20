/*
 * SPDX-FileCopyrightText: 2023 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/uart.h"

#include <errno.h>
#include <string.h>

#include "driver/uart.h"

#define MAX_UART		2

#define DEFAULT_BUFSIZE		256
#define DEFAULT_RXQUEUE_LEN	8

struct uart {
	struct uart_api api;
	struct uart_pin pin;
	uint8_t channel;
	uint32_t baudrate;
	bool activated;
};

static int write_uart(struct uart *self, const void *data, size_t data_len)
{
	if (!self || !self->activated) {
		return -EPIPE;
	}

	if (data_len == 1 && ((const char *)data)[0] == '\n') {
		if (uart_write_bytes(self->channel, " \b\n", 3) < 0) {
			return -EIO;
		}
		return 1;
	}

	int written = uart_write_bytes(self->channel, data, data_len);

	if (written < 0) {
		return -EIO;
	}

	return written;
}

static int read_uart(struct uart *self, void *buf, size_t bufsize)
{
	if (!self || !self->activated) {
		return -EPIPE;
	}

	int len = uart_read_bytes(self->channel,
			   buf, bufsize, (TickType_t)portMAX_DELAY);

	if (len < 0) {
		return -EIO;
	}

	return len;
}

static int enable_uart(struct uart *self, uint32_t baudrate)
{
	if (!self) {
		return -EPIPE;
	} else if (self->activated) {
		return -EALREADY;
	}

	uart_config_t uart_config = {
		.baud_rate = baudrate,
		.data_bits = UART_DATA_8_BITS,
		.parity    = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
		.source_clk = UART_SCLK_DEFAULT,
#endif
	};

	if (uart_driver_install(self->channel, DEFAULT_BUFSIZE*2,
					DEFAULT_BUFSIZE*2, DEFAULT_RXQUEUE_LEN,
					NULL, 0) != ESP_OK) {
		return -EFAULT;
	}

	self->baudrate = baudrate;
	self->activated = true;

	if (uart_param_config(self->channel, &uart_config) != ESP_OK ||
			uart_set_pin(self->channel, self->pin.tx, self->pin.rx,
				self->pin.rts, self->pin.cts) != ESP_OK) {
		return -EINVAL;
	}

	return 0;
}

static int disable_uart(struct uart *self)
{
	if (!self) {
		return -EPIPE;
	} else if (!self->activated) {
		return -EALREADY;
	}

	if (uart_driver_delete(self->channel) != ESP_OK) {
		return -EFAULT;
	}

	self->activated = false;

	return 0;
}

struct uart *uart_create(uint8_t channel, const struct uart_pin *pin)
{
	static struct uart uart[MAX_UART];

	if (channel >= MAX_UART || uart[channel].activated) {
		return NULL;
	}

	uart[channel].api = (struct uart_api) {
		.enable = enable_uart,
		.disable = disable_uart,
		.write = write_uart,
		.read = read_uart,
	};

	uart[channel].channel = channel;

	if (pin) {
		memcpy(&uart[channel].pin, pin, sizeof(*pin));
	}

	return &uart[channel];
}

void uart_delete(struct uart *self)
{
	memset(self, 0, sizeof(*self));
}
