/*
 * SPDX-FileCopyrightText: 2018 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/xmodem.h"

#include <errno.h>
#include <stdbool.h>

#define META_LEN		4

#define DATA_LEN_128		128
#define DATA_LEN_1K		1024
#define DEFAULT_DATA_LEN	DATA_LEN_128

#define TIMEOUT_SYNC_MS		(60 * 1000)
#define TIMEOUT_ACK_MS		(10 * 1000)
#define TIMEOUT_ONE_CHAR_MS	(1 * 1000)

#define IO_TIMEOUT		TIMEOUT_ONE_CHAR_MS

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
	xmodem_flush_t flush;

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

static void do_flush(void)
{
	if (m.flush) {
		m.flush();
	}
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
		bool use_crc, uint32_t timeout_ms)
{
	const uint32_t t_started = millis();
	uint32_t t_now = t_started;
	uint32_t t_sent = t_started - TIMEOUT_ONE_CHAR_MS;

	do {
		if (is_timedout(t_now, t_sent, TIMEOUT_ONE_CHAR_MS)) {
			const uint8_t initiator = use_crc? IDL : NAK;
			do_send(&initiator, 1, IO_TIMEOUT);
			t_sent = t_now;
		}

		if (read_byte(&packet->header) == 1) {
			packet->received = 1;
			return true;
		}

		t_now = millis();
	} while (!is_timedout(t_now, t_started, timeout_ms));

	return false;
}

static uint16_t calculate_crc(const uint8_t *data, size_t datasize)
{
	uint16_t crc = 0;

	for (size_t i = 0; i < datasize; i++) {
		crc ^= (uint16_t)data[i] << 8;
		for (int j = 0; j < 8; j++) {
			if (crc & 0x8000) {
				crc = (uint16_t)(crc << 1 ^ 0x1021);
			} else {
				crc <<= 1;
			}
		}
	}

	return crc;
}

static bool is_packet_valid(const struct packet *packet, bool use_crc)
{
	const size_t data_size = packet->received - META_LEN - (use_crc? 1 : 0);

	if (packet->header == EOT || packet->header == CAN) {
		return true;
	} else if (packet->received < META_LEN + (use_crc? 1 : 0)) {
		return false;
	}

	if (use_crc) {
		const uint16_t c1 = calculate_crc(packet->data, data_size);
		const uint16_t c2 = (uint16_t)
			(packet->chksum[0] << 8 | packet->chksum[1]);
		return c1 == c2;
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
		if ((uint8_t)~packet->seq != packet->seq_inverted) {
			return XMODEM_ERROR_RETRY;
		}
	}
	return XMODEM_ERROR_RECEIVING;
}

static xmodem_error_t read_packet(struct packet *packet,
		size_t data_size, bool use_crc, uint32_t timeout_ms)
{
	const uint32_t t_started = millis();
	size_t *index = &packet->received;
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
			if (is_timedout(t_now, t_read, TIMEOUT_ONE_CHAR_MS)) {
				return XMODEM_ERROR_NO_INPUT;
			}
			continue;
		}

		t_read = t_now;
		fill_buffer(packet, ch, *index, data_size);
		err = check_buffer(packet, *index);

		if (err == XMODEM_ERROR_RETRY) {
			do_flush();
			return err;
		} else if (err == XMODEM_ERROR_RECEIVING) {
			*index = *index + 1;
			if (*index >= (META_LEN + data_size + (use_crc? 1:0))) {
				return XMODEM_ERROR_NONE;
			}
		} else {
			return err;
		}
	} while (!is_timedout(t_now, t_started, timeout_ms));

	return err;
}

static xmodem_error_t process_packet(struct packet *packet, bool use_crc)
{
	if (!is_packet_valid(packet, use_crc)) {
		packet->received = 0;
		send_nak();
		return XMODEM_ERROR_RETRY;
	}

	xmodem_error_t err = XMODEM_ERROR_RECEIVING;

	if (packet->seq == packet->seq_expected || packet->header == EOT) {
		err = XMODEM_ERROR_NONE;
		packet->seq_expected = (uint8_t)((packet->seq + 1) % 256);
	}

	packet->received = 0;
	send_ack();
	return err;
}

xmodem_error_t xmodem_receive(xmodem_data_block_size_t block_type,
		xmodem_recv_callback_t on_recv, void *on_recv_ctx,
		uint32_t timeout_ms)
{
	const uint32_t t_started = millis();
	struct packet packet = { .data = m.buf, .seq_expected = 1};
	size_t data_size = DEFAULT_DATA_LEN;
	size_t nr_packets = 0;
	const bool use_crc = block_type == XMODEM_DATA_BLOCK_128? false : true;
	uint32_t t_now;
	xmodem_error_t err;

	if (packet.data == NULL) {
		return XMODEM_ERROR_INVALID_PARAM;
	}

	if (block_type == XMODEM_DATA_BLOCK_1K) {
		data_size = DATA_LEN_1K;
	}
	if (data_size > m.bufsize) {
		return XMODEM_ERROR_NO_ENOUGH_BUFFER;
	}

	if (!sync_sender(&packet, use_crc, MIN(timeout_ms, TIMEOUT_SYNC_MS))) {
		return XMODEM_ERROR_NO_RESPONSE;
	}

	do {
		t_now = millis();

		if ((err = read_packet(&packet,
					data_size, use_crc, TIMEOUT_ACK_MS))
				== XMODEM_ERROR_CANCELED
				|| err == XMODEM_ERROR_CANCELED_BY_REMOTE) {
			break;
		}

		if (process_packet(&packet, use_crc) != XMODEM_ERROR_NONE) {
			continue;
		}

		nr_packets += 1;

		if (on_recv) {
			if (packet.header == EOT) {
				(*on_recv)(0, 0, 0, on_recv_ctx);
			} else {
				err = (*on_recv)(nr_packets, packet.data,
						data_size, on_recv_ctx);
				if (err != XMODEM_ERROR_NONE) {
					break;
				}
			}
		}
	} while (!is_timedout(t_now, t_started, timeout_ms) &&
			!is_last_packet(&packet));

	if (!is_last_packet(&packet)) {
		send_can();
		err = err == XMODEM_ERROR_NONE? XMODEM_ERROR_TIMEOUT : err;
	}

	return err;
}

void xmodem_set_io(xmodem_reader_t reader, xmodem_writer_t writer,
		xmodem_flush_t flush)
{
	m.reader = reader;
	m.writer = writer;
	m.flush = flush;
}

void xmodem_set_rx_buffer(uint8_t *rx_buf, size_t rx_bufsize)
{
	m.buf = rx_buf;
	m.bufsize = rx_bufsize;
}

void xmodem_set_millis(xmodem_millis_t fn)
{
	m.millis = fn;
}
