COMPONENT_NAME = base64

SRC_FILES = \
	../components/common/src/base64.c

TEST_SRC_FILES = \
	src/common/test_base64.cpp

include test_runners/MakefileRunner.mk
