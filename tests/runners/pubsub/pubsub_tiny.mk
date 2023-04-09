# SPDX-License-Identifier: MIT

COMPONENT_NAME = pubsub_tiny

SRC_FILES = \
	../modules/pubsub_tiny/src/pubsub_tiny.c \
	stubs/logging.c

TEST_SRC_FILES = \
	src/pubsub_tiny/test_pubsub_tiny.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	stubs \
	../modules/pubsub_tiny/include \
	../modules/common/include \
	../modules/logging/include \
	$(CPPUTEST_HOME)/include \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS =

include runners/MakefileRunner
