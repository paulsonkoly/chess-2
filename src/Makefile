version=$(shell                                             \
  if [ "`git rev-parse --abbrev-ref HEAD`" = main ]; then   \
    git rev-parse HEAD;                                     \
  else                                                      \
    git rev-parse --abbrev-ref HEAD;                        \
  fi                                                        \
)

CFLAGS=-DVERSION="$(version)"

include ../makefile.mk

buildpreobj: uci.yy.o

cleanextras:
	rm -f uci.yy.c uci.yy.o

