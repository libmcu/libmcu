COMPONENT_NAME = pubsub_tiny

SRC_FILES = \
	../components/pubsub_tiny/src/pubsub_tiny.c \
	stubs/logging.c

TEST_SRC_FILES = \
	src/pubsub_tiny/test_pubsub_tiny.cpp

INCLUDE_DIRS += \
	stubs \
	../components/logging/include \
	../components/pubsub/include

include test_runners/MakefileRunner.mk
