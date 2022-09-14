# SPDX-License-Identifier: MIT

ifndef SILENCE
SILENCE = @
endif

export TEST_TARGET = $(TEST_BUILDIR)/$(COMPONENT_NAME)_tests
export CPPUTEST_OBJS_DIR = $(TEST_BUILDIR)/objs
export CPPUTEST_LIB_DIR = $(TEST_BUILDIR)/lib

export CPPUTEST_USE_EXTENSIONS = Y
export CPPUTEST_USE_MEM_LEAK_DETECTION = Y
export CPPUTEST_USE_GCOV = Y
export GCOV_ARGS = -abcfpu
export CPPUTEST_EXE_FLAGS = "-c" # colorize output

export CPPUTEST_WARNINGFLAGS = \
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
	-Wunused-macros \
	-Wshadow \
	-Wwrite-strings \
	-Waggregate-return \
	-Wconversion \
	-Wstrict-overflow=5 \
	-Werror \
	\
	-Wswitch-default \
	-Wswitch-enum \
	-Wno-long-long \
	-Wno-missing-braces \
	-Wno-missing-field-initializers \
	-Wno-packed \
	-Wno-unused-parameter \
	\
	-Wno-error=switch-enum \
	-Wno-error=aggregate-return \

#-Wredundant-decls -Wswitch-enum

ifeq ($(shell uname), Darwin)
CPPUTEST_WARNINGFLAGS += \
	-Wno-error=zero-as-null-pointer-constant \
	-Wno-error=poison-system-directories \
	-Wno-error=covered-switch-default \
	-Wno-error=format-nonliteral \
	-Wno-error=pedantic
else
#TARGET_PLATFORM ?= $(shell gcc -dumpmachine)
endif

include $(CPPUTEST_HOME)/build/MakefileWorker.mk
