COMPONENT_NAME = pubsub

SRC_FILES = \
	stubs/logging.c \
	../components/pubsub/src/pubsub.c

TEST_SRC_FILES = \
	src/pubsub/test_pubsub.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	stubs \
	../components/logging/include \
	../components/pubsub/include \
	../components/common/include \
	$(CPPUTEST_HOME)/include \

CPPUTEST_CPPFLAGS = -DPUBSUB_MIN_SUBSCRIPTION_CAPACITY=1

MOCKS_SRC_DIRS =

include MakefileRunner.mk
