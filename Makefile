#
# Makefile
# Adrian Perez, 2010-08-09 20:45
#

CFLAGS += -Os -g -W -Wall -Werror

all:

include Makefile.libwheel

EX_SRCS  := $(wildcard examples/*.c)
EX_OBJS  := $(patsubst %.c,%.o,$(EX_SRCS))
EXAMPLES := $(patsubst %.c,%,$(EX_SRCS))

all: $(libwheel)

examples: $(EXAMPLES)
examples: $(libwheel)

examples/%: examples/%.o $(libwheel)
	$(CC) -o $@ $^


clean: clean-examples

clean-examples:
	$(RM) $(EXAMPLES)
	$(RM) $(EX_OBJS)

.PHONY: examples clean-examples

# vim:ft=make
#

