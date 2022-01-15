BUILDROOT=..
LDFLAGS=-lcmocka

build: test

include ../makefile.mk

test: $(obj)
	@cd $(BUILDROOT)/src && make
	$(CC) $(LDFLAGS) -o $@ $^ $(BUILDROOT)/src/*.o

.PHONY: clean
clean:
	rm -f $(obj) test
