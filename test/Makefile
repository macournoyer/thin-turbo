CC = gcc
CFLAGS = -fno-common -g -O2 -pipe -I../ext/thin_backend -DDEBUG

OBJS = $(addprefix ../ext/thin_backend/,palloc.o array.o buffer.o queue.o)
TARGETS = $(addsuffix _test,palloc array status buffer queue)

%: %.c
	$(CC) $(CFLAGS) -c $<
	$(CC) $(CFLAGS) $(OBJS) $@.o -o $@

all: $(TARGETS)

clean:
	rm -rf $(TARGETS) $(addsuffix .o,$(TARGETS))

test: all
	$(foreach cmd,$(TARGETS),./$(cmd);)

.PHONY: all clean test