src = $(wildcard *.c)
obj = $(src:.c=.o)
dep = $(obj:.o=.d)  # one dependency file for each source
CFLAGS=-Wall -Werror -pedantic -O2 -g -I$(BUILDROOT)/src

build: $(obj)

-include $(dep)   # include all dep files in the makefile

# rule to generate a dep file by using the C preprocessor
# (see man cpp for details on the -MM and -MT options)
%.d: %.c
	@$(CC) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

.PHONY: cleandep
cleandep:
	rm -f $(dep)
