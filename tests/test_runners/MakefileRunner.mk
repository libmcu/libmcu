ifndef SILENCE
SILENCE = @
endif

CPPUTEST_HOME ?= /usr
ifneq ($(shell uname), Darwin)
TARGET_PLATFORM ?= $(shell gcc -dumpmachine)
endif

TEST_SRC_FILES += test_runners/test_all.cpp
MOCKS_SRC_DIRS += mocks

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
	-Wno-error=poison-system-directories \
	-Wno-error=c++11-extensions \
	-Werror

include ../external/cpputest/build/MakefileWorker.mk
