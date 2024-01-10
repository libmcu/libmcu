#include "CppUTestExt/MockSupport.h"
#include "libmcu/spi.h"
#include "bytearray.h"

struct spi {
	struct spi_api api;
};

static int enable_spi(struct spi *self) {
	return mock().actualCall("spi_enable").withParameter("self", self)
		.returnIntValue();
}

static int disable_spi(struct spi *self) {
	return mock().actualCall("spi_disable").withParameter("self", self)
		.returnIntValue();
}

static int write_spi(struct spi *self, const void *data, size_t data_len) {
	return mock().actualCall("spi_write")
		.withParameter("self", self)
		.withParameter("data", data)
		.withParameter("data_len", data_len)
		.returnIntValue();
}

static int read_spi(struct spi *self, void *buf, size_t bufsize) {
	return mock().actualCall("spi_read")
		.withParameter("self", self)
		.withParameter("buf", buf)
		.withParameter("bufsize", bufsize)
		.returnIntValue();
}

static int writeread(struct spi *self, const void *txdata, size_t txdata_len,
		void *rxbuf, size_t rxbuf_len) {
	ByteArray txPtrLen = {
		txdata, txdata_len
	};
	return mock().actualCall("spi_writeread")
		.withParameter("self", self)
		.withParameterOfType("ByteArray", "ptrlen", &txPtrLen)
		.withOutputParameter("rxbuf", rxbuf)
		.withParameter("rxbuf_len", rxbuf_len)
		.returnIntValue();
}

struct spi *spi_create(uint8_t channel) {
	(void)channel;

	static struct spi spi = {
		.api = {
			.enable = enable_spi,
			.disable = disable_spi,
			.write = write_spi,
			.read = read_spi,
			.writeread = writeread,
		},
	};

	return &spi;
}

void spi_delete(struct spi *self) {
	mock().actualCall(__func__).withParameter("self", self);
}
