#include "CppUTestExt/MockSupport.h"
#include "libmcu/spi.h"
#include "bytearray.h"

struct lm_spi *lm_spi_create(uint8_t channel, const struct lm_spi_pin *pin) {
	return (struct lm_spi *)mock().actualCall("lm_spi_create")
		.withIntParameter("channel", channel)
		.returnPointerValue();
}

void lm_spi_destroy(struct lm_spi *self) {
	mock().actualCall("lm_spi_destroy")
		.withPointerParameter("self", self);
}

struct lm_spi_device *lm_spi_create_device(struct lm_spi *self,
		lm_spi_mode_t mode, uint32_t freq_hz, int pin_cs) {
	return (struct lm_spi_device *)mock().actualCall("lm_spi_create_device")
		.withPointerParameter("self", self)
		.withIntParameter("mode", mode)
		.withUnsignedIntParameter("freq_hz", freq_hz)
		.withIntParameter("pin_cs", pin_cs)
		.returnPointerValue();
}

int lm_spi_delete_device(struct lm_spi_device *dev) {
	return mock().actualCall("lm_spi_delete_device")
		.withPointerParameter("dev", dev)
		.returnIntValue();
}

int lm_spi_enable(struct lm_spi_device *dev) {
	return mock().actualCall("lm_spi_enable")
		.withPointerParameter("dev", dev)
		.returnIntValue();
}

int lm_spi_disable(struct lm_spi_device *dev) {
	return mock().actualCall("lm_spi_disable")
		.withPointerParameter("dev", dev)
		.returnIntValue();
}

int lm_spi_write(struct lm_spi_device *dev, const void *data, size_t data_len) {
	return mock().actualCall("lm_spi_write")
		.withPointerParameter("dev", dev)
		.withMemoryBufferParameter("data", (const uint8_t *)data, data_len)
		.returnIntValue();
}

int lm_spi_read(struct lm_spi_device *dev, void *buf, size_t rx_len) {
	return mock().actualCall("lm_spi_read")
		.withPointerParameter("dev", dev)
		.withOutputParameter("buf", buf)
		.returnIntValue();
}

int lm_spi_writeread(struct lm_spi_device *dev,
		const void *txdata, size_t txdata_len,
		void *rxbuf, size_t rx_len) {
	return mock().actualCall("lm_spi_writeread")
		.withPointerParameter("dev", dev)
		.withMemoryBufferParameter("txdata", (const uint8_t *)txdata, txdata_len)
		.withOutputParameter("rxbuf", rxbuf)
		.returnIntValue();
}
