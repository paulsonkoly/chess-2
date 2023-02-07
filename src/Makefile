src = uci.yy.c

iwyu:version.h

include ../makefile.mk

version=$(shell                                             \
  if [ "`git rev-parse --abbrev-ref HEAD`" == main ]; then  \
    git rev-parse HEAD;                                     \
  else                                                      \
    git rev-parse --abbrev-ref HEAD;                        \
  fi                                                        \
)

uci.o: version.h

.ONESHELL: version.h
.PHONY: version.h
version.h:
	@echo Version: $(version)
	@cat << EOS > version.h
	@#ifndef __VERSION_H__
	@#define __VERSION_H__
	@  static const char * version = "$(version)";
	@#endif
	@EOS

.PHONY: clean
clean:
	rm -f $(obj) uci.yy.c version.h
