export BUILDIR ?= build
export INCLUDE_DIRS += ../include

TESTS := $(wildcard test_runners/*_runner.mk)

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
lcov: $(TESTS)
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
        -o $(BUILDIR)/test_coverage/

.PHONY: clean
clean:
	rm -rf $(BUILDIR)
