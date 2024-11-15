/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "CppUTest/TestHarness.h"
#include "CppUTest/TestHarness_c.h"
#include "CppUTestExt/MockSupport.h"

#include "libmcu/xmodem.h"

static int xmodem_read(uint8_t *buf, size_t bufsize, uint32_t timeout_ms) {
	return mock().actualCall(__func__)
		.withOutputParameter("buf", buf)
		.withParameter("bufsize", bufsize)
		.withParameter("timeout_ms", timeout_ms)
		.returnIntValueOrDefault(0);
}

static int xmodem_write(const uint8_t *data, size_t datasize,
		uint32_t timeout_ms) {
	return mock().actualCall(__func__)
		.withMemoryBufferParameter("data", data, datasize)
		.withParameter("timeout_ms", timeout_ms)
		.returnIntValueOrDefault(0);
}

static uint32_t millis(void) {
	return mock().actualCall(__func__)
		.returnUnsignedIntValueOrDefault(0);
}

TEST_GROUP(XMODEM) {
	uint8_t buf[1024];

	void setup(void) {
		xmodem_set_io(xmodem_read, xmodem_write, NULL);
		xmodem_set_millis(millis);
		xmodem_set_rx_buffer(buf, sizeof(buf));
	}
	void teardown(void) {
		mock().checkExpectations();
		mock().clear();
	}

	void expect_sync_with_no_response(uint32_t start_ms, uint32_t step_ms,
			uint32_t count, const uint8_t *data, size_t datasize) {
		for (uint32_t i = 0; i < count; i++) {
			const uint32_t t = start_ms + step_ms * (i+1);
			mock().expectOneCall("xmodem_write")
				.withMemoryBufferParameter("data", data, datasize)
				.withParameter("timeout_ms", 1000);
			mock().expectOneCall("xmodem_read")
				.ignoreOtherParameters()
				.andReturnValue(0);
			mock().expectNCalls(1, "millis").andReturnValue(t);
		}
	}

	void expect_sync(const char syn[1]) {
		mock().expectNCalls(2, "millis").andReturnValue(0);
		mock().expectOneCall("xmodem_write")
			.withMemoryBufferParameter("data", (const uint8_t *)syn, 1)
			.withParameter("timeout_ms", 1000);
	}

	void expect_packet_without_stx(const char seq[1], const char seq_inv[1],
			const char chksum[1], size_t data_count, uint32_t t = 0) {
		mock().expectOneCall("millis").andReturnValue(t);
		mock().expectOneCall("xmodem_read")
			.withOutputParameterReturning("buf", (const uint8_t *)seq, 1)
			.ignoreOtherParameters()
			.andReturnValue(1);
		mock().expectOneCall("millis").andReturnValue(t);
		mock().expectOneCall("xmodem_read")
			.withOutputParameterReturning("buf", (const uint8_t *)seq_inv, 1)
			.ignoreOtherParameters()
			.andReturnValue(1);

		for (size_t i = 0; i < data_count; i++) {
			mock().expectOneCall("millis").andReturnValue(t);
			mock().expectOneCall("xmodem_read")
				.withOutputParameterReturning("buf", (const uint8_t *)"\x00", 1)
				.ignoreOtherParameters()
				.andReturnValue(1);
		}

		mock().expectOneCall("millis").andReturnValue(t);
		mock().expectOneCall("xmodem_read")
			.withOutputParameterReturning("buf", (const uint8_t *)chksum, 1)
			.ignoreOtherParameters()
			.andReturnValue(1);
	}

	void expect_packet(const char stx[1], const char seq[1],
			const char seq_inv[1],
			const char chksum[1], size_t data_count, uint32_t t = 0) {
		mock().expectNCalls(3, "millis").andReturnValue(t);
		mock().expectOneCall("xmodem_read")
			.withOutputParameterReturning("buf", (const uint8_t *)stx, 1)
			.ignoreOtherParameters()
			.andReturnValue(1);
		expect_packet_without_stx(seq, seq_inv, chksum, data_count, t);
	}

	void expect_packet_crc(const char stx[1], const char seq[1], const char seq_inv[1],
			const char crc1[1], const char crc2[1],
			size_t data_count, uint32_t t = 0) {
		expect_packet(stx, seq, seq_inv, crc1, data_count, t);
		mock().expectOneCall("millis").andReturnValue(t);
		mock().expectOneCall("xmodem_read")
			.withOutputParameterReturning("buf", (const uint8_t *)crc2, 1)
			.ignoreOtherParameters()
			.andReturnValue(1);
	}

	void expect_packet_crc_without_stx(const char seq[1], const char seq_inv[1],
			const char crc1[1], const char crc2[1],
			size_t data_count, uint32_t t = 0) {
		mock().expectNCalls(2, "millis").andReturnValue(t);
		expect_packet_without_stx(seq, seq_inv, crc1, data_count, t);
		mock().expectOneCall("millis").andReturnValue(t);
		mock().expectOneCall("xmodem_read")
			.withOutputParameterReturning("buf", (const uint8_t *)crc2, 1)
			.ignoreOtherParameters()
			.andReturnValue(1);
	}

	void expect_ack(void) {
		mock().expectOneCall("xmodem_write")
			.withMemoryBufferParameter("data", (const uint8_t *)"\x06", 1)
			.withParameter("timeout_ms", 1000);
	}

	void expect_nak(void) {
		mock().expectOneCall("xmodem_write")
			.withMemoryBufferParameter("data", (const uint8_t *)"\x15", 1)
			.withParameter("timeout_ms", 1000);
	}

	void expect_c(void) {
		mock().expectOneCall("xmodem_write")
			.withMemoryBufferParameter("data", (const uint8_t *)"\x43", 1)
			.withParameter("timeout_ms", 1000);
	}

	void expect_stx(const char stx[1]) {
		mock().expectOneCall("xmodem_read")
			.withOutputParameterReturning("buf", (const uint8_t *)stx, 1)
			.ignoreOtherParameters()
			.andReturnValue(1);
	}

	void expect_remote_cancel(void) {
		mock().expectOneCall("xmodem_read")
			.withOutputParameterReturning("buf", (const uint8_t *)"\x18", 1)
			.ignoreOtherParameters()
			.andReturnValue(1);
	}

	void expect_eot(uint32_t t = 0) {
		mock().expectNCalls(3, "millis").andReturnValue(t);
		mock().expectOneCall("xmodem_read")
			.withOutputParameterReturning("buf", (const uint8_t *)"\x04", 1)
			.ignoreOtherParameters()
			.andReturnValue(1);
	}
};

TEST(XMODEM, ShouldReceiverSendNAK_WhenNotCRCSupported) {
	mock().expectNCalls(2, "millis").andReturnValue(0);
	expect_sync_with_no_response(0, 1000, 10, (const uint8_t[]){ 0x15 }, 1);

	LONGS_EQUAL(XMODEM_ERROR_NO_RESPONSE,
			xmodem_receive(XMODEM_DATA_BLOCK_128, 0, 0, 10000));
}

TEST(XMODEM, ShouldReceiverSendC_WhenCRCSupported) {
	mock().expectNCalls(2, "millis").andReturnValue(0);
	expect_sync_with_no_response(0, 1000, 10, (const uint8_t[]){ 'C' }, 1);

	LONGS_EQUAL(XMODEM_ERROR_NO_RESPONSE,
			xmodem_receive(XMODEM_DATA_BLOCK_128_CRC, 0, 0, 10000));
}

TEST(XMODEM, ShouldReceiverSendC_When1KCRCSupported) {
	mock().expectNCalls(2, "millis").andReturnValue(0);
	expect_sync_with_no_response(0, 1000, 10, (const uint8_t[]){ 'C' }, 1);

	LONGS_EQUAL(XMODEM_ERROR_NO_RESPONSE,
			xmodem_receive(XMODEM_DATA_BLOCK_1K, 0, 0, 10000));
}

TEST(XMODEM, ShouldReceivePacketSuccessfully_WhenValidPacketReceived) {
	expect_sync("\x15");
	expect_stx("\x01");

	mock().expectNCalls(2, "millis").andReturnValue(0);
	expect_packet_without_stx("\x01", "\xFE", "\x00", 128);
	expect_ack();

	expect_eot();
	expect_ack();

	LONGS_EQUAL(XMODEM_ERROR_NONE,
			xmodem_receive(XMODEM_DATA_BLOCK_128, 0, 0, 10000));
}

TEST(XMODEM, ShouldReceivePacketWith16bitCRC_WhenValidPacketReceived) {
	expect_sync("\x43");
	expect_stx("\x01");
	expect_packet_crc_without_stx("\x01", "\xFE", "\x00", "\x00", 128);
	expect_ack();
	expect_eot();
	expect_ack();
	LONGS_EQUAL(XMODEM_ERROR_NONE,
			xmodem_receive(XMODEM_DATA_BLOCK_128_CRC, 0, 0, 10000));

	expect_sync("\x43");
	expect_stx("\x01");
	expect_packet_crc_without_stx("\x01", "\xFE", "\x00", "\x00", 1024);
	expect_ack();
	expect_eot();
	expect_ack();
	LONGS_EQUAL(XMODEM_ERROR_NONE,
			xmodem_receive(XMODEM_DATA_BLOCK_1K, 0, 0, 10000));
}

TEST(XMODEM, ShouldSendNAK_WhenNoCharacterReceivedForOneSecond) {
	expect_sync("\x43");
	expect_stx("\x01");
	mock().expectNCalls(2, "millis").andReturnValue(0);
	mock().expectOneCall("millis").andReturnValue(1000);
	mock().expectOneCall("xmodem_read")
		.ignoreOtherParameters()
		.andReturnValue(0);
	expect_nak();

	expect_packet_crc("\x01", "\x01", "\xFE", "\x00", "\x00", 128, 1000);
	expect_ack();

	expect_eot(1000);
	expect_ack();

	LONGS_EQUAL(XMODEM_ERROR_NONE,
	     xmodem_receive(XMODEM_DATA_BLOCK_128_CRC, 0, 0, 10000));
}

TEST(XMODEM, ShouldReturnInvalidParam_WhenBufferIsNull) {
	mock().expectOneCall("millis").andReturnValue(0);
	xmodem_set_rx_buffer(NULL, 0);
	LONGS_EQUAL(XMODEM_ERROR_INVALID_PARAM,
			xmodem_receive(XMODEM_DATA_BLOCK_128, 0, 0, 10000));
}

TEST(XMODEM, ShouldReturnNoEnoughBuffer_WhenBufferIsTooSmall) {
	uint8_t buf[16];
	mock().expectOneCall("millis").andReturnValue(0);
	xmodem_set_rx_buffer(buf, sizeof(buf));
	LONGS_EQUAL(XMODEM_ERROR_NO_ENOUGH_BUFFER,
			xmodem_receive(XMODEM_DATA_BLOCK_128, 0, 0, 10000));
}

TEST(XMODEM, ShouldReturnCaneledByRemote_WhenRemoteSendCAN) {
	expect_sync("\x15");
	expect_stx("\x01");

	mock().expectNCalls(2, "millis").andReturnValue(0);
	expect_packet_without_stx("\x01", "\xFE", "\x00", 128);
	expect_ack();

	mock().expectNCalls(3, "millis").andReturnValue(0);
	expect_remote_cancel();
	LONGS_EQUAL(XMODEM_ERROR_CANCELED_BY_REMOTE,
			xmodem_receive(XMODEM_DATA_BLOCK_128, 0, 0, 10000));
}
