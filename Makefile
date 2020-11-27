PROJECT := libmcu
VERSION ?= $(shell git describe --dirty --always --tags)
BASEDIR := $(shell pwd)
BUILDIR := build

Q ?= @
ifneq ($(Q),@)
	override undefine Q
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
CFLAGS += -std=gnu99 \
	  -fno-builtin \
	  -fno-common \
	  -ffunction-sections \
	  -fdata-sections \
	  -Os
#CFLAGS += -fno-short-enums -fstack-usage
#CFLAGS += -flto # it increases stack usage and removes some debug info

## Compiler warnings
CFLAGS += -Wall \
	  -Wextra \
	  -Werror \
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
	  -Wstrict-overflow=5
 #-Wformat-truncation=2 -Wformat-overflow -Wabi=11 -Wlogical-op -Wstack-usage=$(STACK_LIMIT)

## Compiler errors
CFLAGS += -Wno-error=pedantic \
	  -Wno-error=inline \
	  -Wno-error=sign-conversion \
	  -Wno-error=aggregate-return \
	  -Wno-error=unused-parameter \
	  -Wno-error=conversion \
	  -Wno-error=deprecated-declarations \
	  -Wno-error=unused-function \
	  -Wno-error=strict-overflow \
	  -Wno-main
ifndef NDEBUG
	CFLAGS += -g3
	#CFLAGS += -fsanitize=address
endif

# Linker options
LIBS =
LD_SCRIPT =
ifneq ($(LD_SCRIPT),)
	LDFLAGS += -T$(LD_SCRIPT)
endif
ifdef LD_LIBRARY_PATH
	LDFLAGS += -L$(LD_LIBRARY_PATH) -lc
endif

# Build options
SRCDIRS = src
APP_INCLUDES = include tests/stubs
APP_DEFINES  =
SRCS = $(foreach dir, $(SRCDIRS), $(shell find $(dir) -type f -regex ".*\.c"))
INCS = $(addprefix -I, $(APP_INCLUDES))
DEFS = $(addprefix -D, $(APP_DEFINES))
OBJS = $(addprefix $(BUILDIR)/, $(SRCS:.c=.o))
DEPS = $(OBJS:.o=.d)

.DEFAULT_GOAL :=
all: $(OBJS)
	@printf "\n"
	@printf "  $(PROJECT)_$(VERSION)\n"

$(OBJS): $(BUILDIR)/%.o: %.c Makefile $(LD_SCRIPT)
	@printf "  CC       $<\n"
	@mkdir -p $(@D)
	$(Q)$(CC) -o $@ -c $< -MMD $(DEFS) $(INCS) $(CFLAGS)

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
