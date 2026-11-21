# SPDX-License-Identifier: MIT

COMPONENT_NAME = esp_idf_adc_range

# The selector is header-only. Put main in the runner's required archive.
SRC_FILES = src/test_all.cpp
TEST_SRC_FILES = src/esp-idf/adc_range_test.cpp

INCLUDE_DIRS = \
	../ports/esp-idf \
	$(CPPUTEST_HOME)/include

include runners/MakefileRunner
