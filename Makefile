# This Works is placed under the terms of the Copyright Less License,
# see file COPYRIGHT.CLL.  USE AT OWN RISK, ABSOLUTELY NO WARRANTY.

CFLAGS=-Wall -O3 -D_GNU_SOURCE

PROGS=suid
INSTPATH=/usr/local/bin

.PHONY: all
all:	$(PROGS)

.PHONY: install
install: $(PROGS)
	mkdir -p '$(INSTPATH)'
	strip -s $(PROGS)
	cp -f $(PROGS) '$(INSTPATH)/'
	cd '$(INSTPATH)' && chown root:root $(PROGS)
	cd '$(INSTPATH)' && chmod 7555 $(PROGS)

.PHONY: clean distclean
clean distclean:
	rm -f $(PROGS) *.o *~

