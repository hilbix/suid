# This Works is placed under the terms of the Copyright Less License,
# see file COPYRIGHT.CLL.  USE AT OWN RISK, ABSOLUTELY NO WARRANTY.

CFLAGS=-Wall -O3 -D_GNU_SOURCE

PROGS=suid
CONF=suid.conf
INSTPATH=/usr/local/bin

.PHONY: all
all:	$(PROGS)

suid.o:	suid.c oops.h args.h linereader.h

.PHONY: install
install: $(PROGS)
	mkdir -p '$(INSTPATH)'
	strip -s $(PROGS)
	cp -f $(PROGS) '$(INSTPATH)/'
	cd '$(INSTPATH)' && chown root:root $(PROGS)
	cd '$(INSTPATH)' && chmod 4555 $(PROGS)
	if [ -f '/etc/$(CONF)' ]; then cp -v '$(CONF)' '/etc/$(CONF).dist'; else cp -v '$(CONF)' '/etc/$(CONF)'; fi

.PHONY: clean distclean
clean distclean:
	rm -f $(PROGS) *.o *~

