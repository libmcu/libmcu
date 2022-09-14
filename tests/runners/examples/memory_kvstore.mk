# SPDX-License-Identifier: MIT

COMPONENT_NAME = memory_kvstore

SRC_FILES = \
	../examples/memory_kvstore.c

TEST_SRC_FILES = \
	src/examples/memory_kvstore_test.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	../examples \
	../modules/common/include \
	$(CPPUTEST_HOME)/include \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS =

include MakefileRunner.mk
