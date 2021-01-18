COMPONENT_NAME = pubsub

SRC_FILES = \
	stubs/logging.c \
	../components/pubsub/src/pubsub.c

TEST_SRC_FILES = \
	src/pubsub/test_pubsub.cpp

INCLUDE_DIRS += \
	stubs \
	../components/logging/include \
	../components/pubsub/include

CPPUTEST_CPPFLAGS += -DPUBSUB_MIN_SUBSCRIPTION_CAPACITY=1

include test_runners/MakefileRunner.mk
