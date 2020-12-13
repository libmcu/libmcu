COMPONENT_NAME = logging_storage

SRC_FILES = \
	../examples/logging/memory_storage.c \
	../examples/ringbuf.c \
	../examples/logging/fake_storage.c

TEST_SRC_FILES = \
	src/test_logging_memory_storage.cpp \
	src/test_logging_fake_storage.cpp

INCLUDE_DIRS += \
	../examples \
	../examples/logging

include test_runners/MakefileRunner.mk
