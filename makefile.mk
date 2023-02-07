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

# unfortunately github doesn't have iwyu 0.19 which can control exit status
.ONESHELL: %.iwyu
.PHONY: %.iwyu
%.iwyu: %.c
	if include-what-you-use -Wall -Werror -pedantic -O2 -g -I./src $< 2>&1 | grep -E 'should (add|remove)' > /dev/null; then
	include-what-you-use -Wall -Werror -pedantic -O2 -g -I./src $< 2>&1
	exit 1
	fi

.PHONY: iwyu
iwyu: $(iwyus)

