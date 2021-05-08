export CPPUTEST_HOME = cpputest
export BUILDIR ?= build
export INCLUDE_DIRS += \
	../components/common/include/libmcu/posix \
	../components/common/include \
	. \
	$(CPPUTEST_HOME)/include

TESTS := $(shell find test_runners -type f -regex ".*runner\.mk")

.PHONY: all
all: test
.PHONY: test
test: BUILD_RULE=all
test: $(TESTS)
.PHONY: compile
compile: BUILD_RULE=start
compile: $(TESTS)
.PHONY: $(TESTS)
$(TESTS):
	$(MAKE) -f $@ $(BUILD_RULE)

LCOV_INFO_FILE = $(BUILDIR)/lcov.info
.PHONY: lcov
$(BUILDIR)/test_coverage lcov: $(BUILDIR)
	@lcov \
        --base-directory . \
        --directory . \
        -c -o $(LCOV_INFO_FILE).tmp
	@lcov \
        --remove $(LCOV_INFO_FILE).tmp \
        -o $(LCOV_INFO_FILE) \
        '*cpputest/*' \
        '/usr/*' \
        '*tests/*'
	@genhtml \
        -o test_coverage \
        -t "coverage" \
        --num-spaces 4 \
        $(LCOV_INFO_FILE) \
        -o $@

.PHONY: coverage
coverage: $(BUILDIR)/test_coverage
	$(Q)open $(BUILDIR)/test_coverage/index.html
$(BUILDIR): $(TESTS)

.PHONY: clean
clean:
	rm -rf $(BUILDIR)

.PHONY: install reinstall
install: | $(CPPUTEST_HOME)
	@cd $(CPPUTEST_HOME) && autoreconf -i && ./configure && make
reinstall:
	@cd $(CPPUTEST_HOME) && make
$(CPPUTEST_HOME):
	@git clone -b hack/malloc --single-branch \
		https://github.com/onkwon/cpputest.git $@
