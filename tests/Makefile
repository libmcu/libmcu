# SPDX-License-Identifier: MIT

export CPPUTEST_HOME = cpputest
export TEST_BUILDIR ?= build

TESTS := $(shell find runners -type f -regex ".*\.mk")

.PHONY: all test compile gcov debug flags
all: test
test: BUILD_RULE=all
test: $(TESTS)
compile: BUILD_RULE=start
compile: $(TESTS)
gcov: BUILD_RULE=gcov
gcov: $(TESTS)
debug: BUILD_RULE=debug
debug: $(TESTS)
flags: BUILD_RULE=flags
flags: $(TESTS)

.PHONY: $(TESTS)
$(TESTS):
	$(MAKE) -f $@ $(BUILD_RULE)

COVERAGE_FILE = $(TEST_BUILDIR)/coverage.info
.PHONY: $(COVERAGE_FILE) $(COVERAGE_FILE).init $(COVERAGE_FILE).run
$(COVERAGE_FILE).init: $(TEST_BUILDIR)
	@lcov -o $@ \
		--base-directory . --directory . -c -i
$(COVERAGE_FILE).run:
	@lcov -o $@ \
		--base-directory . --directory . -c
$(COVERAGE_FILE): $(COVERAGE_FILE).init $(COVERAGE_FILE).run
	@lcov -o $@ \
		-a $(COVERAGE_FILE).init -a $(COVERAGE_FILE).run
	@lcov -o $@ \
		--remove $@ \
		'*cpputest/*' \
		'/usr/*' \
		'*tests/*' \
		'*external/*'
$(TEST_BUILDIR)/test_coverage: $(COVERAGE_FILE)
	@genhtml \
		-o $@ \
		-t "coverage" \
		--num-spaces 4 \
		$(COVERAGE_FILE)

.PHONY: coverage
coverage: $(TEST_BUILDIR)/test_coverage
	$(Q)open $(TEST_BUILDIR)/test_coverage/index.html
$(TEST_BUILDIR): $(TESTS)

.PHONY: clean
clean:
	$(Q)rm -rf $(TEST_BUILDIR)

.PHONY: install reinstall
install: | $(CPPUTEST_HOME)
	@cd $(CPPUTEST_HOME) && autoreconf -i && ./configure && make
reinstall:
	@cd $(CPPUTEST_HOME) && make
$(CPPUTEST_HOME):
	@git clone -b hack/malloc --single-branch \
		https://github.com/onkwon/cpputest.git $@
