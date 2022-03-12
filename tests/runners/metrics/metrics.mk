COMPONENT_NAME = metrics

SRC_FILES = \
	stubs/logging.c \
	../modules/metrics/src/metrics.c

TEST_SRC_FILES = \
	src/metrics/test_metrics.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	stubs/overrides \
	src/metrics \
	../modules/logging/include \
	../modules/metrics/include \
	../modules/common/include \
	$(CPPUTEST_HOME)/include \

CPPUTEST_CPPFLAGS = \
	-DMETRICS_USER_DEFINES=\"my_metrics.def\" \
	-DMETRICS_KEY_STRING \
	-DLIBMCU_NOINIT=

MOCKS_SRC_DIRS =

include MakefileRunner.mk
