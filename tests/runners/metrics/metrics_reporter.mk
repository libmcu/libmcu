# SPDX-License-Identifier: MIT

COMPONENT_NAME = metrics_reporter

SRC_FILES = \
	../modules/metrics/src/metrics_reporter.c \

TEST_SRC_FILES = \
	src/metrics/test_metrics_reporter.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	src/metrics \
	../modules/metrics/include \
	../modules/common/include \
	../interfaces/kvstore/include \
	stubs/overrides \
	$(CPPUTEST_HOME)/include \

CPPUTEST_CPPFLAGS = \
	-DMETRICS_USER_DEFINES=\"my_metrics.def\" \
	-DLIBMCU_NOINIT=

MOCKS_SRC_DIRS =

include runners/MakefileRunner
