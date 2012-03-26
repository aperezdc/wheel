#
# Makefile
# Adrian Perez, 2010-08-09 20:45
#

CFLAGS += -O0 -D_DEBUG -g -W -Wall -Werror

all:

include Makefile.libwheel

EX_SRCS  := $(wildcard examples/*.c)
EX_OBJS  := $(patsubst %.c,%.o,$(EX_SRCS))
EXAMPLES := $(patsubst %.c,%,$(EX_SRCS))

all: $(libwheel)

examples: $(EXAMPLES)
examples: $(libwheel)

examples/%: examples/%.o $(libwheel)
	$(CC) $(LDFLAGS) -o $@ $^ -lm


clean: clean-examples

clean-examples:
	$(RM) $(EXAMPLES)
	$(RM) $(EX_OBJS)

.PHONY: examples clean-examples


## Unit tests

libwheel_CHECKS  := $(wildcard tests/check-*.c)
libwheel_TESTS   := $(patsubst tests/check-%.c,tests/test-%.c,$(libwheel_CHECKS))
libwheel_TESTS_O := $(patsubst %.c,%.all.o,$(libwheel_TESTS))
libwheel_TESTRUN := $(patsubst %.c,%,$(libwheel_TESTS))

tests/test-%.c: tests/check-%.c
	./tests/maketest $< > $@

tests/test-all.c: $(libwheel_TESTS)
	./tests/makealltests $^ > $@

tests/%.all.o: tests/%.c
	$(COMPILE.c) $(OUTPUT_OPTION) $<

tests: tests/test-all
all-tests: $(libwheel_TESTRUN)
$(libwheel_TESTRUN): $(libwheel) -lcheck -lm
$(libwheel_TESTS): $(libwheel_PATH)/wheel.h
tests/test-all: CPPFLAGS += -DTEST_NO_MAIN
tests/test-all: tests/test-all.o $(libwheel_TESTS_O) $(libwheel) -lcheck -lm

clean: clean-tests

clean-tests:
	$(RM) $(libwheel_TESTRUN)
	$(RM) $(libwheel_TESTS:.c=.o)
	$(RM) $(libwheel_TESTS_O)
	$(RM) $(libwheel_TESTS)
	$(RM) tests/test-all.c tests/test-all.o tests/test-all

run-all-tests: all-tests
	@for test in $(libwheel_TESTRUN) ; do \
		"$${test}" || break ; \
	done

run-tests: tests/test-all examples/wcfg-to-tnetstring
	@./tests/test-tnetstr
	@./tests/test-all

.PHONY: tests clean-tests run-tests all-tests

.SECONDARY:

# vim:ft=make
#

