COMPONENT_NAME = metrics

SRC_FILES = \
	stubs/logging.c \
	../components/metrics/src/metrics.c

TEST_SRC_FILES = \
	src/metrics/test_metrics.cpp

INCLUDE_DIRS += \
	stubs \
	../components/logging/include \
	../components/metrics/include

include test_runners/MakefileRunner.mk
