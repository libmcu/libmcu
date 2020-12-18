COMPONENT_NAME = pubsub

SRC_FILES = \
	    ../src/pubsub.c \
	    stubs/logging.c

TEST_SRC_FILES = \
	    src/test_pubsub.cpp

INCLUDE_DIRS += stubs
CPPUTEST_CPPFLAGS += -DPUBSUB_MIN_SUBSCRIPTION_CAPACITY=1

include test_runners/MakefileRunner.mk
