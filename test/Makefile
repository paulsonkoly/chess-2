BUILDROOT=..
CFLAGS+=$(shell pkg-config --cflags cmocka)
LDFLAGS=$(shell pkg-config --libs cmocka)

build: test

include ../makefile.mk

test: $(obj)
	@cd $(BUILDROOT)/src && make
	$(CC) -o $@ $^ $(BUILDROOT)/src/*.o $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) test
