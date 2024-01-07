# SPDX-License-Identifier: MIT

COMPONENT_NAME = FLASH_KVSTORE

SRC_FILES = \
	../ports/kvstore/flash_kvstore.c \
	../modules/common/src/hash.c

TEST_SRC_FILES = \
	src/common/flash_kvstore_test.cpp \
	stubs/logging.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	../modules/common/include \
	../modules/logging/include \
	../interfaces/flash/include \
	../tests/mocks \
	$(CPPUTEST_HOME)/include \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS = -DUNITTEST -include libmcu/logging.h \
		    -DACTOR_DEBUG=debug -DACTOR_INFO=info -DACTOR_WARN=warn

include runners/MakefileRunner
