# SPDX-License-Identifier: MIT

ifneq ($(LIBMCU_ROOT),)
libmcu-basedir := $(LIBMCU_ROOT)/
endif

LIBMCU_INTERFACES ?= adc apptmr ble flash gpio i2c kvstore l4 pwm spi uart wdt wifi

LIBMCU_INTERFACES_INCS := $(foreach d, $(LIBMCU_INTERFACES), \
	$(addprefix $(libmcu-basedir)interfaces/, $(d))/include)
