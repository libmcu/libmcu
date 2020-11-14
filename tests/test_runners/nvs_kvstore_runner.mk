COMPONENT_NAME = nvs_kvstore

SRC_FILES = \
	../ports/esp-idf/nvs_kvstore.c \
	mocks/mock_nvs.cpp

TEST_SRC_FILES = \
	src/test_nvs_kvstore.cpp

INCLUDE_DIRS += stubs

include test_runners/MakefileRunner.mk
