BUILDROOT=..
CFLAGS+=$(shell pkg-config --cflags cmocka)
LDFLAGS=$(shell pkg-config --libs cmocka)

build: engineobjs test

include ../makefile.mk

test: $(obj) .engineobjs
	$(CC) -o $@ $^ $(BUILDROOT)/src/*.o $(LDFLAGS)

.PHONY: engineobjs
engineobjs:
	@cd $(BUILDROOT)/src && make
	touch .engineobjs

.PHONY: clean
clean:
	rm -f $(obj) .engineobjs test
