/*
 * SPDX-FileCopyrightText: 2023 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#define UART_PARITY_EVEN	LIBMCU_UART_PARITY_EVEN
#define UART_PARITY_ODD		LIBMCU_UART_PARITY_ODD
#define uart_parity_t		libmcu_uart_parity_t
#define uart_flush		libmcu_uart_flush
#include "libmcu/uart.h"
#undef uart_flush
#undef uart_parity_t
#undef UART_PARITY_ODD
#undef UART_PARITY_EVEN

#include <errno.h>
#include <string.h>

#include "driver/uart.h"

#define MAX_UART			2

#define DEFAULT_BUFSIZE			256
#define DEFAULT_RXQUEUE_LEN		8

#if !defined(UART_DEFAULT_OPTION_PARITY)
#define UART_DEFAULT_OPTION_PARITY	UART_PARITY_NONE
#endif
#if !defined(UART_DEFAULT_OPTION_DATABIT)
#define UART_DEFAULT_OPTION_DATABIT	8
#endif
#if !defined(UART_DEFAULT_OPTION_STOPBIT)
#define UART_DEFAULT_OPTION_STOPBIT	UART_STOPBIT_1
#endif
#if !defined(UART_DEFAULT_OPTION_FLOWCTRL)
#define UART_DEFAULT_OPTION_FLOWCTRL	UART_FLOWCTRL_NONE
#endif
#if !defined(UART_DEFAULT_OPTION_RX_TIMEOUT)
#define UART_DEFAULT_OPTION_RX_TIMEOUT	-1
#endif

struct uart {
	struct uart_config config;
	struct uart_pin pin;
	uint8_t channel;
	bool activated;
};

static void set_config(uart_config_t *uart_config,
		const struct uart_config *config)
{
	uart_config->baud_rate = config->baudrate;

	switch (config->databit) {
	case 5:
		uart_config->data_bits = UART_DATA_5_BITS;
		break;
	case 6:
		uart_config->data_bits = UART_DATA_6_BITS;
		break;
	case 7:
		uart_config->data_bits = UART_DATA_7_BITS;
		break;
	case 8: /* fall through */
	default:
		uart_config->data_bits = UART_DATA_8_BITS;
		break;
	}

	if (config->parity == LIBMCU_UART_PARITY_EVEN) {
		uart_config->parity = UART_PARITY_EVEN;
	} else if (config->parity == LIBMCU_UART_PARITY_ODD) {
		uart_config->parity = UART_PARITY_ODD;
	} else {
		uart_config->parity = UART_PARITY_DISABLE;
	}

	if (config->stopbit == UART_STOPBIT_1_5) {
		uart_config->stop_bits = UART_STOP_BITS_1_5;
	} else if (config->stopbit == UART_STOPBIT_2) {
		uart_config->stop_bits = UART_STOP_BITS_2;
	} else {
		uart_config->stop_bits = UART_STOP_BITS_1;
	}

	switch (config->flowctrl) {
	case UART_FLOWCTRL_RTS:
		uart_config->flow_ctrl = UART_HW_FLOWCTRL_RTS;
		break;
	case UART_FLOWCTRL_CTS:
		uart_config->flow_ctrl = UART_HW_FLOWCTRL_CTS;
		break;
	case UART_FLOWCTRL_CTS_RTS:
		uart_config->flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS;
		break;
	case UART_FLOWCTRL_NONE: /* fall through */
	default:
		uart_config->flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
		break;
	}

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
	uart_config->source_clk = UART_SCLK_DEFAULT;
#endif
}

static int update_config(struct uart *self)
{
	uart_config_t uart_config = { 0, };
	set_config(&uart_config, &self->config);

	if (uart_param_config(self->channel, &uart_config) != ESP_OK) {
		return -EINVAL;
	}

	return 0;
}

int uart_configure(struct uart *self, const struct uart_config *config)
{
	if (!self) {
		return -EPIPE;
	} else if (!config) {
		return -EINVAL;
	}

	const uint32_t baudrate = self->config.baudrate;

	memcpy(&self->config, config, sizeof(*config));

	if (self->config.baudrate == 0) {
		self->config.baudrate = baudrate;
	}

	if (!self->activated) {
		return 0;
	}

	return update_config(self);
}

int uart_write(struct uart *self, const void *data, size_t data_len)
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

int uart_read(struct uart *self, void *buf, size_t bufsize)
{
	if (!self || !self->activated) {
		return -EPIPE;
	}

	int len = uart_read_bytes(self->channel, buf, bufsize,
			pdMS_TO_TICKS(self->config.rx_timeout_ms));

	if (len < 0) {
		return -EIO;
	}

	return len;
}

int libmcu_uart_flush(struct uart *self)
{
	if (!self || !self->activated) {
		return -EPIPE;
	}

	if (uart_wait_tx_done(self->channel, -1) != ESP_OK) {
		return -EIO;
	}

	return 0;
}

int uart_clear(struct uart *self)
{
	if (!self || !self->activated) {
		return -EPIPE;
	}

	if (uart_flush_input(self->channel) != ESP_OK) {
		return -EIO;
	}

	return 0;
}

int uart_enable(struct uart *self, uint32_t baudrate)
{
	if (!self) {
		return -EPIPE;
	} else if (self->activated) {
		return -EALREADY;
	}

	if (uart_driver_install(self->channel, DEFAULT_BUFSIZE*2,
					DEFAULT_BUFSIZE*2, DEFAULT_RXQUEUE_LEN,
					NULL, 0) != ESP_OK) {
		return -EFAULT;
	}

	self->activated = true;
	self->config.baudrate = baudrate;

	if (uart_set_pin(self->channel, self->pin.tx, self->pin.rx,
				self->pin.rts, self->pin.cts) != ESP_OK ||
			update_config(self) != 0) {
		return -EINVAL;
	}

	return 0;
}

int uart_disable(struct uart *self)
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

	uart[channel].channel = channel;
	uart[channel].config = (struct uart_config) {
		.databit = UART_DEFAULT_OPTION_DATABIT,
		.parity = UART_DEFAULT_OPTION_PARITY,
		.stopbit = UART_DEFAULT_OPTION_STOPBIT,
		.flowctrl = UART_DEFAULT_OPTION_FLOWCTRL,
		.rx_timeout_ms = UART_DEFAULT_OPTION_RX_TIMEOUT,
	};

	if (pin) {
		memcpy(&uart[channel].pin, pin, sizeof(*pin));
	}

	return &uart[channel];
}

void uart_delete(struct uart *self)
{
	memset(self, 0, sizeof(*self));
}
