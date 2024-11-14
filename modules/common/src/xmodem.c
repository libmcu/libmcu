/*
 * SPDX-FileCopyrightText: 2018 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/xmodem.h"
#include "libmcu/crc16.h"
#include <errno.h>
#include <stdbool.h>

#define META_SIZE		4

#define DATA_SIZE_128		128
#define DATA_SIZE_1K		1024
#define DEFAULT_DATA_SIZE	DATA_SIZE_128

#define TIMEOUT_SYNC_SEC	60
#define TIMEOUT_ACK_SEC		10
#define TIMEOUT_ONE_CHAR_SEC	1
#define TIMEOUT_INIT_SEC	3

#define IO_TIMEOUT		(TIMEOUT_ONE_CHAR_SEC * 1000)

#if !defined(MIN)
#define MIN(a, b)		((a) < (b)? (a) : (b))
#endif

enum {
	SOH			= 0x01,
	STX			= 0x02,
	ETX			= 0x03,
	EOT			= 0x04,
	ACK			= 0x06,
	DLE			= 0x10,
	NAK			= 0x15,
	CAN			= 0x18,
	SUB			= 0x1A,
	IDL			= 0x43, /* 'C' */
};

struct packet {
	uint8_t header;
	uint8_t seq;
	uint8_t seq_inverted;
	uint8_t *data;
	uint8_t chksum[2];

	size_t received;
	uint8_t seq_expected;
};

static struct {
	xmodem_reader_t reader;
	xmodem_writer_t writer;

	xmodem_millis_t millis;

	uint8_t *buf;
	size_t bufsize;
} m;

static uint32_t millis(void)
{
	if (m.millis == NULL) {
		return 0;
	}

	return m.millis();
}

static int do_send(const uint8_t *data, size_t datasize, uint32_t timeout_ms)
{
	if (m.writer == NULL) {
		return -ENODEV;
	}

	return m.writer(data, datasize, timeout_ms);
}

static int do_read(uint8_t *buf, size_t bufsize, uint32_t timeout_ms)
{
	if (m.reader == NULL) {
		return -ENODEV;
	}

	return m.reader(buf, bufsize, timeout_ms);
}

static int read_byte(uint8_t *ch)
{
	return do_read(ch, 1, IO_TIMEOUT);
}

static int send_can(void)
{
	uint8_t tx[] = { CAN, CAN };
	return do_send(tx, sizeof(tx), IO_TIMEOUT);
}

static int send_nak(void)
{
	uint8_t tx = NAK;
	return do_send(&tx, sizeof(tx), IO_TIMEOUT);
}

static int send_ack(void)
{
	uint8_t tx = ACK;
	return do_send(&tx, sizeof(tx), IO_TIMEOUT);
}

static bool is_timedout(uint32_t now, uint32_t start, uint32_t timeout)
{
	return (now - start) >= timeout;
}

static bool is_last_packet(const struct packet *packet)
{
	return packet->header == EOT || packet->header == CAN;
}

static bool sync_sender(struct packet *packet,
		uint8_t *initiator, uint32_t timeout_ms)
{
	const uint32_t t_started = millis();
	uint32_t t_now = t_started;
	uint32_t t_sent = t_started - TIMEOUT_ONE_CHAR_SEC * 1000;
	size_t count = 0;

	do {
		if (is_timedout(t_now, t_sent, TIMEOUT_ONE_CHAR_SEC * 1000)) {
			if (*initiator == IDL && count >
					(TIMEOUT_ACK_SEC / TIMEOUT_INIT_SEC)) {
				*initiator = NAK;
			}
			do_send(initiator, 1, IO_TIMEOUT);
			t_sent = t_now;
			count += 1;
		}

		if (read_byte(&packet->header) == 1) {
			packet->received = 1;
			return true;
		}

		t_now = millis();
	} while (!is_timedout(t_now, t_started, timeout_ms));

	return false;
}

static bool is_packet_valid(const struct packet *packet, bool crc)
{
	const size_t data_size = packet->received - META_SIZE - (crc? 1 : 0);

	if (packet->header == EOT || packet->header == CAN) {
		return true;
	} else if (packet->received < META_SIZE + (crc? 1 : 0)) {
		return false;
	}

	if (crc) {
		uint16_t c = crc16_xmodem(packet->data, data_size);
		return c == (packet->chksum[0] << 8 | packet->chksum[1]);
	}

	uint8_t chksum = 0;

	for (size_t i = 0; i < data_size; i++) {
		chksum += packet->data[i];
	}

	return chksum == packet->chksum[0];
}

static void fill_buffer(struct packet *packet,
		uint8_t ch, size_t index, size_t data_size)
{
	if (index < 3) { /* header */
		uint8_t *p = (uint8_t *)packet;
		p[index] = ch;
	} else if (index >= 3 && (index - 3) < data_size) { /* data */
		packet->data[index - 3] = ch;
	} else if ((index - 3) >= data_size) { /* checksum */
		packet->chksum[index - 3 - data_size] = ch;
	}
}

static xmodem_error_t check_buffer(const struct packet *packet, size_t index)
{
	if (index == 0) {
		if (packet->header == EOT) {
			return XMODEM_ERROR_NONE;
		} else if (packet->header == CAN) {
			return XMODEM_ERROR_CANCELED_BY_REMOTE;
		} else if (packet->header != SOH && packet->header != STX) {
			return XMODEM_ERROR_RETRY;
		}
	} else if (index == 2) {
		if ((uint8_t)~packet->seq != (uint8_t)packet->seq_inverted) {
			return XMODEM_ERROR_RETRY;
		}
	}
	return XMODEM_ERROR_RECEIVING;
}

static xmodem_error_t read_packet(struct packet *packet,
		size_t data_size, bool crc, uint32_t timeout_ms)
{
	const uint32_t t_started = millis();
	size_t *index = (size_t *)&packet->received;
	xmodem_error_t err = XMODEM_ERROR_TIMEOUT;
	uint32_t t_read = t_started;
	uint32_t t_now;
	uint8_t ch;

	if (*index == 1 && packet->header != SOH && packet->header != STX) {
		*index = 0;
	}

	do {
		t_now = millis();

		if (read_byte(&ch) != 1) {
			if (is_timedout(t_now, t_read,
					TIMEOUT_ONE_CHAR_SEC * 1000)) {
				send_nak();
				return XMODEM_ERROR_NO_INPUT;
			}
			continue;
		}

		t_read = t_now;
		fill_buffer(packet, ch, *index, data_size);
		err = check_buffer(packet, *index);

		if (err == XMODEM_ERROR_RETRY) {
			continue;
		} else if (err == XMODEM_ERROR_RECEIVING) {
			*index = *index + 1;
			if (*index >= (META_SIZE + data_size + (crc? 1 : 0))) {
				return XMODEM_ERROR_NONE;
			}
		} else {
			return err;
		}
	} while (!is_timedout(t_now, t_started, timeout_ms));

	return err;
}

xmodem_error_t xmodem_receive(xmodem_data_block_size_t block_type,
		xmodem_recv_callback_t on_recv, void *on_recv_ctx,
		uint32_t timeout_ms)
{
	const uint32_t t_started = millis();
	struct packet packet = { .data = m.buf, .seq_expected = 1};
	size_t data_size = DEFAULT_DATA_SIZE;
	size_t nr_packets = 0;
	uint8_t initiator = block_type == XMODEM_DATA_BLOCK_128? NAK : IDL;
	uint32_t t_now;
	xmodem_error_t err;

	if (packet.data == NULL) {
		return XMODEM_ERROR_INVALID_PARAM;
	}

	if (!sync_sender(&packet,
			&initiator, MIN(timeout_ms, TIMEOUT_SYNC_SEC*1000))) {
		return XMODEM_ERROR_NO_RESPONSE;
	}

	if (initiator == IDL && block_type == XMODEM_DATA_BLOCK_1K) {
		data_size = DATA_SIZE_1K;
	}

	if (data_size > m.bufsize) {
		return XMODEM_ERROR_NO_ENOUGH_BUFFER;
	}

	do {
		t_now = millis();
		err = read_packet(&packet, data_size, initiator == IDL,
				TIMEOUT_ACK_SEC * 1000);

		if (err == XMODEM_ERROR_NONE &&
				is_packet_valid(&packet, initiator == IDL)) {
			if (packet.seq == packet.seq_expected) {
				packet.seq_expected = (uint8_t)
					((packet.seq + 1) % 256);
				nr_packets += 1;
				if (on_recv) {
					(*on_recv)(packet.header == EOT?
							0 : nr_packets,
							packet.data, data_size,
							on_recv_ctx);
				}
			}
			packet.received = 0;
			send_ack();
			continue;
		}

		packet.received = 0;
		send_nak();
	} while (!is_timedout(t_now, t_started, timeout_ms) &&
			!is_last_packet(&packet));

	if (!is_last_packet(&packet)) {
		send_can();
		err = err == XMODEM_ERROR_NONE? XMODEM_ERROR_TIMEOUT : err;
	}

	return err;
}

void xmodem_set_io(xmodem_reader_t reader, xmodem_writer_t writer)
{
	m.reader = reader;
	m.writer = writer;
}

void xmodem_set_rx_buffer(uint8_t *rx_buf, size_t rx_bufsize)
{
	m.buf = rx_buf;
	m.bufsize = rx_bufsize;
}

void xmodem_set_millis(xmodem_millis_t millis)
{
	m.millis = millis;
}
