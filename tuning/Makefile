CFLAGS=-DTUNING -I../src -I.

include ../makefile.mk

buildpreobj: evaluate.o srcbuild

.PHONY:
srcbuild:
	cd ../src && make && cd ..

evaluate.o: ../src/evaluate.c
	$(CC) -c $(CFLAGS) $^ -o $@

buildpostobj: tuning

srcobjs=$(filter-out ../src/evaluate.o ../src/tuned_values.o, $(wildcard ../src/*.o))

tuning: $(obj)
	$(CC) $(CFLAGS) -lm -o $@ $^ evaluate.o $(srcobjs)

cleanextras:
	rm -f evaluate.o tuning
