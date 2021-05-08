COMPONENT_NAME = apptimer

SRC_FILES = \
	stubs/logging.c \
	stubs/bitops.c \
	../components/apptimer/src/apptimer.c \

TEST_SRC_FILES = \
	src/apptimer/test_apptimer.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	stubs \
	../components/logging/include \
	../components/apptimer/include \
	../components/common/include \
	$(CPPUTEST_HOME)/include \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS =

include MakefileRunner.mk
