# no suffix rules
.SUFFIXES:

src = $(wildcard *.c)
obj = $(src:.c=.o)
dep = $(obj:.o=.d)  # one dependency file for each source
CFLAGS+=-Wall -Werror -pedantic -O2 -g -DNDEBUG -march=native

##
# build
##
build: buildpreobj $(obj) buildpostobj

buildpreobj: $(dep)

buildpostobj:


# rule to generate a dep file by using the C preprocessor
# (see man cpp for details on the -MM and -MT options)
%.d: %.c
	@$(CC) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

-include $(dep)   # include all dep files in the makefile

%.o: %.d
	$(CC) -c $(CFLAGS) $(@:.o=.c) -o $@

%.yy.c: %.fl
	flex -o $@ $?

##
# Cleaning
##

.PHONY: clean
clean: cleandep cleanobj cleanextras

cleandep:
	rm -f $(dep)

cleanobj:
	rm -f $(obj)

cleanextras:

##
# IWYU
##

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
iwyu: $(iwyus) iwyuextras

iwyuextras:


