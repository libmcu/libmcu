PROJECT := libmcu
BASEDIR := $(shell pwd)
BUILDIR := build
SRCDIRS := src

VERSION := $(shell git describe --long --tags --dirty --always)
VERSION_LIST := $(subst -, , $(VERSION))
VERSION_TAG := $(subst ., , $(subst v,, $(strip $(word 1, $(VERSION_LIST)))))
VERSION_MAJOR := $(strip $(word 1, $(VERSION_TAG)))
VERSION_MINOR := $(strip $(word 2, $(VERSION_TAG)))
VERSION_BUILD := $(strip $(word 2, $(VERSION_LIST)))

V ?= 0
ifeq ($(V), 0)
	Q = @
else
	Q =
endif

# Toolchains
ifneq ($(CROSS_COMPILE),)
	CROSS_COMPILE_PREFIX := $(CROSS_COMPILE)-
endif
CC := $(CROSS_COMPILE_PREFIX)gcc
LD := $(CROSS_COMPILE_PREFIX)ld
SZ := $(CROSS_COMPILE_PREFIX)size
AR := $(CROSS_COMPILE_PREFIX)ar
OC := $(CROSS_COMPILE_PREFIX)objcopy
OD := $(CROSS_COMPILE_PREFIX)objdump

# Compiler options
CFLAGS += \
	  -std=gnu99 \
	  -static \
	  -nostdlib \
	  -fno-builtin \
	  -fno-common \
	  -ffunction-sections \
	  -fdata-sections \
	  -Os
#CFLAGS += -fstack-usage # generates .su files per each objects
#CFLAGS += -fno-short-enums
#CFLAGS += -flto # it increases stack usage and removes some debug info

## Compiler warnings
CFLAGS += \
	  -Wall \
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
	  -Wshadow \
	  -Wwrite-strings \
	  -Waggregate-return \
	  -Wredundant-decls \
	  -Wconversion \
	  -Wstrict-overflow=5 \
	  -Werror
#-Wformat-truncation=2
#-Wformat-overflow
#-Wabi=11 -Wlogical-op -Wstack-usage=$(STACK_LIMIT)

## Compiler errors
CFLAGS += -Wno-error=format-nonliteral

ifndef NDEBUG
	CFLAGS += -g3
	#CFLAGS += -fsanitize=address
endif

# Linker options
LDFLAGS += \
	   --gc-sections \
	   --print-gc-sections
LIBS =
LD_SCRIPT =
ifneq ($(LD_SCRIPT),)
	LDFLAGS += -T$(LD_SCRIPT)
endif
ifdef LD_LIBRARY_PATH
	LDFLAGS += -L$(LD_LIBRARY_PATH) -lc
endif

# Build options
APP_INCLUDES = include include/libmcu/posix tests/stubs
APP_DEFINES  = \
	       _POSIX_THREADS \
	       BUILD_DATE=\""$(shell date)"\" \
	       VERSION_MAJOR=$(VERSION_MAJOR) \
	       VERSION_MINOR=$(VERSION_MINOR) \
	       VERSION_BUILD=$(VERSION_BUILD) \
	       VERSION=$(VERSION)

SRCS = $(foreach dir, $(SRCDIRS), $(shell find $(dir) -type f -regex ".*\.c"))
INCS = $(addprefix -I, $(APP_INCLUDES))
DEFS = $(addprefix -D, $(APP_DEFINES))
OBJS = $(addprefix $(BUILDIR)/, $(SRCS:.c=.o))
DEPS = $(OBJS:.o=.d)

.DEFAULT_GOAL :=
all: $(OBJS)
	@echo "\n  $(PROJECT)_$(VERSION)"

$(OBJS): $(BUILDIR)/%.o: %.c Makefile $(LD_SCRIPT)
	@echo "  CC       $*.c"
	@mkdir -p $(@D)
	$(Q)$(CC) -o $@ -c $*.c -MMD $(DEFS) $(INCS) $(CFLAGS)

ifneq ($(MAKECMDGOALS), clean)
ifneq ($(MAKECMDGOALS), depend)
-include $(DEPS)
endif
endif

.PHONY: test
test:
	$(Q)$(MAKE) -C tests
.PHONY: coverage
coverage:
	$(Q)$(MAKE) -C tests $@
.PHONY: clean
clean:
	$(Q)$(MAKE) -C tests clean
	$(Q)rm -rf $(BUILDIR)
