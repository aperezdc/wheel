#
# Makefile
# Adrian Perez, 2010-08-09 20:45
#

CFLAGS += -W -Wall -Werror -fPIC

SRCS  := $(wildcard *.c)
OBJS  := $(patsubst %.c,%.o,$(SRCS))

EX_SRCS  := $(wildcard examples/*.c)
EX_OBJS  := $(patsubst %.c,%.o,$(EX_SRCS))
EXAMPLES := $(patsubst %.c,%,$(EX_SRCS))

all: libwheel.a libwheel.so

examples: $(EXAMPLES)
examples: libwheel.a
examples: CPPFLAGS += -I.

examples/%: examples/%.o libwheel.a
	$(CC) -o $@ $^

libwheel.so: $(OBJS)
	$(CC) -shared -o $@ $^
	-chmod -x $@

libwheel.a: $(OBJS)
	$(AR) rscu $@ $^

clean:
	$(RM) libwheel.a libwheel.so
	$(RM) $(OBJS)
	$(RM) $(EXAMPLES)
	$(RM) $(EX_OBJS)

.PHONY: examples

# vim:ft=make
#

