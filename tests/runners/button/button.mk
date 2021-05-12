COMPONENT_NAME = button

SRC_FILES = \
	../components/button/src/button.c

TEST_SRC_FILES = \
	src/button/button_test.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	stubs/overrides \
	../components/button/include \
	../components/common/include \
	$(CPPUTEST_HOME)/include \

CPPUTEST_CPPFLAGS = -DBUTTON_MAX=3

include MakefileRunner.mk
