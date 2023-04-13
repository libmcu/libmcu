# SPDX-License-Identifier: MIT

ifneq ($(LIBMCU_ROOT),)
libmcu-basedir := $(LIBMCU_ROOT)/
endif

LIBMCU_INTERFACES_SRCS := $(foreach d, \
	$(addprefix $(libmcu-basedir)interfaces/, $(LIBMCU_INTERFACES)), \
	$(shell find $(d)/src -maxdepth 1 -type f -regex ".*\.c"))
LIBMCU_INTERFACES_INCS := $(foreach d, $(LIBMCU_INTERFACES), \
	$(addprefix $(libmcu-basedir)interfaces/, $(d))/include)
