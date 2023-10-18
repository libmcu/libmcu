#include "CppUTestExt/MockSupport.h"
#include "libmcu/spi.h"
#include "bytearray.h"

void spi_delete(struct spi *self) {
	mock().actualCall(__func__).withParameter("self", self);
}

int spi_enable(struct spi *self) {
	return mock().actualCall(__func__).withParameter("self", self)
		.returnIntValue();
}

int spi_disable(struct spi *self) {
	return mock().actualCall(__func__).withParameter("self", self)
		.returnIntValue();
}

int spi_write(struct spi *self, const void *data, size_t data_len) {
	return mock().actualCall(__func__)
		.withParameter("self", self)
		.withParameter("data", data)
		.withParameter("data_len", data_len)
		.returnIntValue();
}

int spi_read(struct spi *self, void *buf, size_t bufsize) {
	return mock().actualCall(__func__)
		.withParameter("self", self)
		.withParameter("buf", buf)
		.withParameter("bufsize", bufsize)
		.returnIntValue();
}

int spi_writeread(struct spi *self, const void *txdata, size_t txdata_len,
		void *rxbuf, size_t rxbuf_len) {
	ByteArray txPtrLen = {
		txdata, txdata_len
	};
	return mock().actualCall(__func__)
		.withParameter("self", self)
		.withParameterOfType("ByteArray", "ptrlen", &txPtrLen)
		.withOutputParameter("rxbuf", rxbuf)
		.withParameter("rxbuf_len", rxbuf_len)
		.returnIntValue();
}
