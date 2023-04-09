# SPDX-License-Identifier: MIT

COMPONENT_NAME = pubsub

SRC_FILES = \
	../modules/pubsub/src/pubsub.c

TEST_SRC_FILES = \
	src/pubsub/pubsub_test.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	stubs/overrides \
	../modules/pubsub/include \
	../modules/common/include \
	$(CPPUTEST_HOME)/include \

CPPUTEST_CPPFLAGS = -DPUBSUB_MIN_SUBSCRIPTION_CAPACITY=1

MOCKS_SRC_DIRS =

include runners/MakefileRunner
