COMPONENT_NAME = button

SRC_FILES = \
	stubs/logging.c \
	../components/button/src/button.c

TEST_SRC_FILES = \
	src/button/test_button.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	../components/logging/include \
	../components/button/include \
	../components/common/include \
	$(CPPUTEST_HOME)/include \

CPPUTEST_CPPFLAGS = -DBUTTON_MAX=3

include MakefileRunner.mk
