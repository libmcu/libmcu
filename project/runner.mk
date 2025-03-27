# SPDX-License-Identifier: MIT

include project/version.mk
include project/toolchain.mk

excludes := trace README.md
LIBMCU_MODULES ?= $(filter-out $(excludes), $(patsubst modules/%, %, $(wildcard modules/*)))
LIBMCU_INTERFACES ?= $(filter-out $(excludes), $(patsubst interfaces/%, %, $(wildcard interfaces/*)))
include project/modules.mk
include project/interfaces.mk
LIBMCU_MODULES_INCS += $(libmcu-basedir)modules/common/include/libmcu/posix

$(info MODULES: $(LIBMCU_MODULES))
$(info INTERFACES: $(LIBMCU_INTERFACES))

SRCS += $(LIBMCU_MODULES_SRCS) $(LIBMCU_INTERFACES_SRCS)
INCS += $(LIBMCU_MODULES_INCS) $(LIBMCU_INTERFACES_INCS)
DEFS += \
	METRICS_USER_DEFINES=\"../metrics.def\" \
	_POSIX_C_SOURCE=200809L \
	_POSIX_THREADS \
	BUILD_DATE=\""$(shell date)"\" \
	VERSION_TAG=$(VERSION_TAG) \
	VERSION=$(VERSION)

OBJS += $(addprefix $(BUILDIR)/, $(SRCS:.c=.o))
DEPS += $(OBJS:.o=.d)

OUTCOM := $(BUILDIR)/$(PROJECT)_$(VERSION_TAG)
OUTLIB := $(OUTCOM).a
OUTZIP := $(OUTCOM).tgz
OUTSHA := $(OUTCOM).sha256
OUTPUT := $(OUTZIP) $(BUILDIR)/sources.txt $(BUILDIR)/includes.txt

all: $(OUTPUT)
	$(Q)$(SZ) -t --common $(sort $(OBJS))

$(BUILDIR)/sources.txt: $(OUTLIB)
	$(info generating  $@)
	$(Q)echo $(sort $(SRCS)) | tr ' ' '\n' > $@
$(BUILDIR)/includes.txt: $(OUTLIB)
	$(info generating  $@)
	$(Q)echo $(subst -I,,$(sort $(INCS))) | tr ' ' '\n' > $@

$(OUTZIP): $(OUTSHA)
	$(info generating  $@)
	$(Q)rm -f $@
	$(Q)tar -zcf $@ $(basename $<).*
$(OUTSHA): $(OUTLIB)
	$(info generating  $@)
	$(Q)openssl dgst -sha256 $< > $@
$(OUTLIB): $(OBJS)
	$(info archiving   $@)
	$(Q)rm -f $@
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
