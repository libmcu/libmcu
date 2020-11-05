ifndef SILENCE
SILENCE = @
endif

TEST_SRC_FILES    += test_runners/test_all.cpp
#MOCKS_SRC_DIRS    += mocks
INCLUDE_DIRS      += fakes spies
CPPUTEST_CPPFLAGS += -DUNITTEST

export TEST_TARGET = $(BUILDIR)/$(COMPONENT_NAME)_tests
export CPPUTEST_OBJS_DIR = $(BUILDIR)/objs
export CPPUTEST_LIB_DIR = $(BUILDIR)/lib

export CPPUTEST_USE_EXTENSIONS=Y
export CPPUTEST_USE_MEM_LEAK_DETECTION=Y
export CPPUTEST_USE_GCOV=Y
export GCOV_ARGS=-b -c # branch coverage report
export CPPUTEST_EXE_FLAGS = "-c" # colorize output

export CPPUTEST_WARNINGFLAGS = \
	-Wall \
	-Wextra \
	-Wshadow \
	-Wswitch-default \
	-Wswitch-enum \
	-Wconversion \
	-Wno-long-long \
	-Wno-missing-braces \
	-Wno-missing-field-initializers \
	-Wno-packed \
	-Wno-unused-parameter \
	\
	-Werror

ifeq ($(shell uname), Darwin)
CPPUTEST_WARNINGFLAGS += \
	-Wno-error=poison-system-directories \
	-Wno-error=c++11-extensions \
	-Wno-error=language-extension-token \
	-Wno-error=covered-switch-default \
	-Wno-error=unused-function
else
#TARGET_PLATFORM ?= $(shell gcc -dumpmachine)
endif

include ../external/cpputest/build/MakefileWorker.mk
