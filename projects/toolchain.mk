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

CFLAGS += -std=gnu99 \
	  -static \
	  -nostdlib \
	  -fno-builtin \
	  -fno-common \
	  -ffunction-sections \
	  -fdata-sections \
	  -fstack-usage \
	  -Os
#CFLAGS += -fno-short-enums

## Compiler warnings
STACK_LIMIT ?= 128
CFLAGS += -Wall \
	  -Wextra \
	  -Wformat=2 \
	  -Wmissing-prototypes \
	  -Wstrict-prototypes \
	  -Wmissing-declarations \
	  -Wcast-align \
	  -Wpointer-arith \
	  -Wbad-function-cast \
	  -Wnested-externs \
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
	  -Wstack-usage=$(STACK_LIMIT)
#-Wformat-truncation=2
#-Wformat-overflow
#-Wabi=11 -Wlogical-op

## Compiler errors
CFLAGS += -Werror \
	  -Wno-error=format-nonliteral

ifndef NDEBUG
	CFLAGS += -g3
	#CFLAGS += -fsanitize=address
endif

# Linker options
LDFLAGS += -Wl,--gc-sections \
	   -Wl,--print-memory-usage
	   #--print-gc-sections
#LDFLAGS += -flto # it increases stack usage and removes some debug info

ARFLAGS = crsu
