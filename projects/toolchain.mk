# SPDX-License-Identifier: MIT

ifneq ($(CROSS_COMPILE),)
	CROSS_COMPILE_PREFIX := $(CROSS_COMPILE)-
endif
CC := $(CROSS_COMPILE_PREFIX)gcc
LD := $(CROSS_COMPILE_PREFIX)ld
SZ := $(CROSS_COMPILE_PREFIX)size
AR := $(CROSS_COMPILE_PREFIX)ar
OC := $(CROSS_COMPILE_PREFIX)objcopy
OD := $(CROSS_COMPILE_PREFIX)objdump
NM := $(CROSS_COMPILE_PREFIX)nm

## Compiler options
LIBMCU_CFLAGS ?= \
	-std=c99 \
	-static \
	-ffreestanding \
	-fno-builtin \
	-fno-common \
	-ffunction-sections \
	-fdata-sections \
	-fstack-usage \
	-Os
	#-fno-short-enums
	#-nostdlib

ifndef NDEBUG
	LIBMCU_CFLAGS += -g3
	#LIBMCU_CFLAGS += -fsanitize=address
endif

## Compiler warnings
STACK_LIMIT ?= 256
LIBMCU_WARNING_FLAGS ?= \
	-Werror \
	-Wall \
	-Wextra \
	-Wpedantic \
	-Wc++-compat \
	-Wformat=2 \
	-Wmissing-prototypes \
	-Wstrict-prototypes \
	-Wmissing-declarations \
	-Wcast-align \
	-Wpointer-arith \
	-Wbad-function-cast \
	-Wcast-qual \
	-Wmissing-format-attribute \
	-Wmissing-include-dirs \
	-Wformat-nonliteral \
	-Wdouble-promotion \
	-Wfloat-equal \
	-Winline \
	-Wundef \
	-Wunused-macros \
	-Wshadow \
	-Wwrite-strings \
	-Waggregate-return \
	-Wredundant-decls \
	-Wconversion \
	-Wstrict-overflow=5 \
	-Wno-long-long \
	-Wswitch-default \
	\
	-Wno-error=format-nonliteral \

	#-Wformat-truncation=2
	#-Wformat-overflow
	#-Wabi=11 -Wlogical-op
	#-Wnested-externs
	#-Wswitch-enum

COMPILER := $(shell $(CC) --version 2>/dev/null | head -n 1 | grep -i clang >/dev/null && echo clang || echo gcc)
ifeq ($(COMPILER), clang)
else
LIBMCU_WARNING_FLAGS += -Wstack-usage=$(STACK_LIMIT) -Wno-error=inline
endif

ifeq ($(shell uname), Darwin)
else
endif

## Linker options
LIBMCU_LDFLAGS ?= \
	-Wl,--gc-sections \
	-Wl,--print-memory-usage
	#--print-gc-sections
	#-flto # it increases stack usage and removes some debug info
	#-specs=nano.specs

## Archiver options
LIBMCU_ARFLAGS ?= crsu

CFLAGS += $(LIBMCU_CFLAGS) $(LIBMCU_WARNING_FLAGS)
LDFLAGS += $(LIBMCU_LDFLAGS)
ARFLAGS = $(LIBMCU_ARFLAGS)
