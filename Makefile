#
# Makefile
# Adrian Perez, 2010-08-09 20:45
#

CFLAGS += -W -Wall -Werror -fPIC

SRCS  := $(wildcard *.c)
OBJS  := $(patsubst %.c,%.o,$(SRCS))

all: libwheel.a libwheel.so

libwheel.so: $(OBJS)
	$(CC) -shared -o $@ $^
	-chmod -x $@

libwheel.a: $(OBJS)
	$(AR) rscu $@ $^

clean:
	$(RM) libwheel.a
	$(RM) $(OBJS)

# vim:ft=make
#

