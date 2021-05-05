ifneq ($(LIBMCU_ROOT),)
libmcu-basedir := $(LIBMCU_ROOT)/
endif

ifeq ($(filter common, $(LIBMCU_COMPONENTS)),)
LIBMCU_COMPONENTS += common
endif

LIBMCU_COMPONENTS_SRCS := $(foreach d, \
	$(addprefix $(libmcu-basedir)components/, $(LIBMCU_COMPONENTS)), \
	$(shell find $(d)/src -maxdepth 1 -type f -regex ".*\.c"))
LIBMCU_COMPONENTS_INCS := $(foreach d, $(LIBMCU_COMPONENTS), \
	$(addprefix $(libmcu-basedir)components/, $(d))/include)
LIBMCU_COMPONENTS_INCS += $(libmcu-basedir)components/common/include/libmcu/posix
