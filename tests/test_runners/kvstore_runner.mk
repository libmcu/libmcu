COMPONENT_NAME = KVStore

SRC_FILES = \
	../src/memory_kvstore.c \
	../ports/esp-idf/nvs_kvstore.c

TEST_SRC_FILES = \
	src/test_memory_kvstore.cpp \
	src/test_nvs_kvstore.cpp

INCLUDE_DIRS += stubs

include test_runners/MakefileRunner.mk
