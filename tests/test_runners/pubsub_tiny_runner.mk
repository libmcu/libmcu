COMPONENT_NAME = pubsub_tiny

SRC_FILES = \
	    ../src/pubsub_tiny.c \
	    stubs/logging.c

TEST_SRC_FILES = \
	    src/test_pubsub_tiny.cpp

INCLUDE_DIRS += stubs

include test_runners/MakefileRunner.mk
