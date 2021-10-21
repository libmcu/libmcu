ifneq ($(LIBMCU_ROOT),)
libmcu-basedir := $(LIBMCU_ROOT)/
endif

ifeq ($(filter common, $(LIBMCU_MODULES)),)
LIBMCU_MODULES += common
endif

LIBMCU_MODULES_SRCS := $(foreach d, \
	$(addprefix $(libmcu-basedir)modules/, $(LIBMCU_MODULES)), \
	$(shell find $(d)/src -maxdepth 1 -type f -regex ".*\.c"))
LIBMCU_MODULES_INCS := $(foreach d, $(LIBMCU_MODULES), \
	$(addprefix $(libmcu-basedir)modules/, $(d))/include)
LIBMCU_MODULES_INCS += $(libmcu-basedir)modules/common/include/libmcu/posix
