#!/usr/bin/make

SUBDIRS = source

all %:
	@for dir in $(SUBDIRS); do \
	$(MAKE) -C $$dir $@ || exit; \
	done

