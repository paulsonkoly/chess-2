.SUFFIXES:

src += $(wildcard *.c)
obj = $(src:.c=.o)
dep = $(obj:.o=.d)  # one dependency file for each source
CFLAGS=-Wall -Werror -pedantic -O2 -g -DNDEBUG -I$(BUILDROOT)/src

build: $(obj)

%.o:
	$(CC) -c $(CFLAGS) $(@:.o=.c) -o $@

-include $(dep)   # include all dep files in the makefile

# rule to generate a dep file by using the C preprocessor
# (see man cpp for details on the -MM and -MT options)
%.d: %.c
	@$(CC) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

%.yy.c: %.fl
	flex -o $@ $?

.PHONY: cleandep
cleandep:
	rm -f $(dep)

iwyus = $(src:.c=.iwyu)

.PHONY: %.iwyu
%.iwyu: %.c
	include-what-you-use -Xiwyu --error=1 $(filter-out -DNDEBUG, $(CFLAGS)) $<

.PHONY: iwyu
iwyu: $(iwyus)

