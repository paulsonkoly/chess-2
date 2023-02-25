CFLAGS=-DTUNING -I../src -I.

include ../makefile.mk

buildpreobj: evaluate.o srcbuild

.PHONY:
srcbuild:
	cd ../src && make && cd ..

evaluate.o: ../src/evaluate.c
	$(CC) -c $(CFLAGS) $^ -o $@

buildpostobj: tuning

srcobjs=../src/board.o ../src/mat_tables.o ../src/pawns.o ../src/moveexec.o ../src/zobrist.o ../src/attacks.o

tuning: $(obj)
	$(CC) $(CFLAGS) -o $@ $^ evaluate.o $(srcobjs)

cleanextras:
	rm -f evaluate.o tuning
