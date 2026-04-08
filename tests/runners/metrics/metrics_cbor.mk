# SPDX-License-Identifier: MIT

COMPONENT_NAME = metrics_cbor

SRC_FILES = \
	stubs/logging.c \
	../modules/metrics/src/metrics.c \
	../ports/metrics/cbor_encoder.c \
	../../cbor/src/common.c \
	../../cbor/src/encoder.c \
	../../cbor/src/ieee754.c \

TEST_SRC_FILES = \
	mocks/assert.cpp \
	src/metrics/test_metrics_cbor.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	src/metrics \
	../modules/logging/include \
	../modules/metrics/include \
	../modules/common/include \
	../../cbor/include \
	stubs/overrides \
	$(CPPUTEST_HOME)/include \

CPPUTEST_CPPFLAGS = \
	-DMETRICS_USER_DEFINES=\"my_metrics.def\" \
	-DMETRICS_KEY_STRING \
	-DLIBMCU_NOINIT= \

MOCKS_SRC_DIRS =

include runners/MakefileRunner
