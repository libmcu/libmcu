PROJECT := libmcu
BASEDIR := $(shell pwd)
BUILDIR := build

VERBOSE ?= 0
V ?= $(VERBOSE)
ifeq ($(V), 0)
	Q = @
else
	Q =
endif
export BASEDIR
export BUILDIR
export Q

include projects/version.mk
include projects/toolchain.mk

LIBMCU_COMPONENTS ?= $(patsubst components/%, %, $(wildcard components/*))
ifeq ($(filter common, $(LIBMCU_COMPONENTS)),)
LIBMCU_COMPONENTS += common
endif

SRCS += $(foreach d, $(addprefix components/, $(LIBMCU_COMPONENTS)), \
	$(shell find $(d)/src -type f -regex ".*\.c"))
INCS += $(foreach d, $(LIBMCU_COMPONENTS), \
	$(addprefix components/, $(d))/include)
INCS += components/common/include/libmcu/posix
DEFS += \
	_POSIX_THREADS \
	BUILD_DATE=\""$(shell date)"\" \
	VERSION_TAG=$(VERSION_TAG) \
	VERSION=$(VERSION)

OBJS += $(addprefix $(BUILDIR)/, $(SRCS:.c=.o))
DEPS += $(OBJS:.o=.d)

OUTLIB := $(BUILDIR)/$(PROJECT).a
OUTPUT := $(OUTLIB) $(BUILDIR)/sources.txt $(BUILDIR)/includes.txt

all: $(OUTPUT)
	$(Q)$(SZ) -t --common $(sort $(OBJS))

$(BUILDIR)/sources.txt: $(OUTLIB)
	$(info generating  $@)
	$(Q)echo $(sort $(SRCS)) | tr ' ' '\n' > $@
$(BUILDIR)/includes.txt: $(OUTLIB)
	$(info generating  $@)
	$(Q)echo $(subst -I,,$(sort $(INCS))) | tr ' ' '\n' > $@
$(OUTLIB): $(OBJS)
	$(info archiving   $@)
	$(Q)$(AR) $(ARFLAGS) $@ $^ 1> /dev/null 2>&1

$(OBJS): $(BUILDIR)/%.o: %.c $(MAKEFILE_LIST)
	$(info compiling   $<)
	@mkdir -p $(@D)
	$(Q)$(CC) -o $@ -c $*.c -MMD \
		$(addprefix -D, $(DEFS)) \
		$(addprefix -I, $(INCS)) \
		$(CFLAGS)

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
