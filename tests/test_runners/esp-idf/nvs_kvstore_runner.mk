COMPONENT_NAME = nvs_kvstore

SRC_FILES = \
	../ports/esp-idf/nvs_kvstore.c \
	mocks/esp-idf/mock_nvs.cpp

TEST_SRC_FILES = \
	src/esp-idf/test_nvs_kvstore.cpp

INCLUDE_DIRS += stubs/esp-idf

include test_runners/MakefileRunner.mk
