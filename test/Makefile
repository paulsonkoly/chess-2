CFLAGS=$(shell pkg-config --cflags cmocka) -I../src/
LDFLAGS=$(shell pkg-config --libs cmocka)

include ../makefile.mk

buildpreobj:
	@cd ../src && make && cd -

buildpostobj: test

test: $(obj)
	$(CC) -o $@ $^ ../src/*.o $(LDFLAGS)

cleanextras:
	rm -f test
