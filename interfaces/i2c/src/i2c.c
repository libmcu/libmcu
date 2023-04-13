#include "libmcu/i2c.h"
#include "libmcu/port/i2c.h"

int i2c_init(struct i2c *self)
{
	return i2c_port_init(self);
}

int i2c_deinit(struct i2c *self)
{
	return i2c_port_deinit(self);
}

int i2c_write(struct i2c *self, uint8_t addr, uint8_t reg,
		const void *data, size_t data_len)
{
	return i2c_port_write(self, addr, reg, data, data_len);
}

int i2c_read(struct i2c *self, uint8_t addr, uint8_t reg,
		void *buf, size_t bufsize)
{
	return i2c_port_read(self, addr, reg, buf, bufsize);
}
