COMPONENT_NAME = pubsub

SRC_FILES = \
	    ../src/pubsub.c

TEST_SRC_FILES = \
	    src/test_pubsub.cpp

INCLUDE_DIRS += stubs

include test_runners/MakefileRunner.mk
