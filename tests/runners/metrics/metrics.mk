COMPONENT_NAME = metrics

SRC_FILES = \
	stubs/logging.c \
	../components/metrics/src/metrics.c

TEST_SRC_FILES = \
	src/metrics/test_metrics.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	src/metrics \
	stubs \
	../components/logging/include \
	../components/metrics/include \
	../components/common/include \
	$(CPPUTEST_HOME)/include \

CPPUTEST_CPPFLAGS = -DMETRICS_USER_DEFINES=\"my_metrics.def\"

MOCKS_SRC_DIRS =

include MakefileRunner.mk
