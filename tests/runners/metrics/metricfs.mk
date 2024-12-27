# SPDX-License-Identifier: MIT

COMPONENT_NAME = metricfs

SRC_FILES = \
	../modules/metrics/src/metricfs.c \

TEST_SRC_FILES = \
	src/metrics/metricfs_test.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	$(CPPUTEST_HOME)/include \
	../modules/metrics/include \
	../interfaces/kvstore/include \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS =

include runners/MakefileRunner
