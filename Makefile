CFLAGS=-I./src

include makefile.mk

buildpreobj:
	cd src && make && cd ..

buildpostobj: chess2

cleanextras:
	cd src && make clean && cd ..
	cd test && make clean && cd ..
	rm -f chess2

iwyuextras:
	cd src && make iwyu && cd ..
	cd tuning && make iwyu && cd ..

chess2: $(obj)
	$(CC) $(CFLAGS) -o $@ $^ src/*.o
