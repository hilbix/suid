# Makefile automatically generated, do not edit!
# This output (only this Makefile) is Public Domain.
#
#@MD5TINOIGN@ Creation date: Wed Jan 16 16:08:11 CET 2019
#
# This file is based on following files:
#@MD5TINOIGN@ 1: Makefile.tino
#@MD5TINOIGN@ 2: /home/tino/src/tinolib/Makefile.proto

#
#@MD5TINOIGN@ included: Makefile.tino
#

      PROGS=suid
       CONF=suid.conf
       OBJS=

 INSTALLBIN=
INSTALLSBIN=
 INSTALLLIB=
 INSTALLETC=
 INSTALLMAN=
INSTALLSMAN=
 INSTALLINF=
INSTALLSINF=

 ADD_CFLAGS=
ADD_CCFLAGS=
ADD_LDFLAGS=
 ADD_LDLIBS=
      CLEAN=
  CLEANDIRS=
  DISTCLEAN=
   TINOCOPY=

INSTALLPATH=/usr/local

#
#@MD5TINOIGN@ included: /home/tino/src/tinolib/Makefile.proto
#

# Automatically generated from "PROGS" above
      PROG1=suid

# Override those in Makefile.tino if needed:
 STD_CFLAGS=-g -Wall -Wno-unused-function -O3 -Wno-error=unused-value -Wno-error=unused-function
 STD_CCFLAGS=-g -Wall -Wno-unused-function -O3 -Wno-error=unused-value -Wno-error=unused-function
STD_LDFLAGS=
 STD_LDLIBS=
    BINPATH=bin
   SBINPATH=sbin
    LIBPATH=lib
    ETCPATH=etc
    MANPATH=man
   SMANPATH=share/man
    INFPATH=info
   SINFPATH=share/info

# Except for the compiler generated dependencies at the end
# from here no changes shall be needed.

     CFLAGS=$(DBG_FLAGS) $(DBG_CFLAGS) $(STD_CFLAGS) $(ADD_CFLAGS) -I"$(HERE)"
   CXXFLAGS=$(DBG_CCFLAGS) $(STD_CCFLAGS) $(ADD_CCFLAGS) -I"$(HERE)"
    LDFLAGS=$(DBG_LDFLAGS) $(STD_LDFLAGS) $(ADD_LDFLAGS)
     LDLIBS=$(DBG_LDLIBS) $(STD_LDLIBS) $(ADD_LDLIBS)

VERSIONFILE=$(PROG1)_version
VERSIONNAME=$(VERSIONFILE)
 VERSIONEXT=h
     COMMON=			\
		$(VERSIONFILE).$(VERSIONEXT)	\
		$(TINOINC)	\
		$(TINOLIB)	\

       GAWK=awk
      TOUCH=touch
      MKDIR=mkdir

         CP=cp
      STRIP=strip

       HERE=$(PWD)

# Semi-automatically generated for CygWin (*.exe)

  PROGS_EXE=			\
		$(PROG1).exe	\

.PHONY: all static install it clean distclean dist tar diff always

# Targets considered to work for all systems with GNU MAKE and GNU AWK

all::	$(SUBDIRS) $(PROGS)

test::	all Tests
	$(PWD)/tino/Makefile-tests.sh Tests

# To use this you need to do:
#	ln -s tinolib/diet .
#	make static
# This is experimental.

static::
	[ -f diet.distignore~ ] || $(MAKE) clean
	$(TOUCH) diet.distignore~
	[ ! -d diet ] || $(MAKE) -C diet diet
	[ ! -d diet ] || $(MAKE) CC="$(PWD)/diet/tinodiet.sh --tinodiet"
	[ -d diet ] || $(MAKE) CC="$(PWD)/tino/Makefile-diet.sh --tinodiet"

Makefile:	Makefile.md5
	$(TOUCH) Makefile

Makefile.md5:	$(VERSIONFILE).$(VERSIONEXT) always
	@[ -z '$(HERE)' ] || $(GAWK) -vHERE="$(HERE)" -vMAKE="$(MAKE)" -vTINOCOPY="$(TINOCOPY)" 'BEGIN { \
	if ((getline < "tino/Makefile")>0 && \
	    (getline < "tino/Makefile.proto")>0 && \
	    (getline < "tino/Makefile.awk")>-1) \
		system(MAKE " -C tino tino HERE='\''" HERE "'\'' TINOCOPY='\''" TINOCOPY "'\''"); \
	else if ((getline < "Makefile.md5")<0)	 \
		printf "" >"Makefile.md5"; \
	}'

$(VERSIONFILE).h:	VERSION
	$(GAWK) -vPROG="$(VERSIONNAME)" '{ gsub(/-/,"_",PROG); print "#define " toupper(PROG) " \"" $$0 "\"" }' VERSION > "$(VERSIONFILE).h"

$(VERSIONFILE).py:	VERSION
	$(GAWK) -vPROG="$(VERSIONNAME)" '{ gsub(/-/,"_",PROG); print "#"; print ""; print toupper(PROG) "=\"" $$0 "\"" }' VERSION > "$(VERSIONFILE).py"

# Poor man's install
# Generated from PROGS and INSTALL* above

install::
	$(RM) "$(INSTALLPATH)/bin/$(PROG1)"
	$(MKDIR) -pm755 "$(INSTALLPATH)/bin"
	$(CP) "$(PROG1)" "$(INSTALLPATH)/bin/$(PROG1)"
	$(STRIP) "$(INSTALLPATH)/bin/$(PROG1)"

# Special targets considered to work for all unix like systems
# like CygWin

it::	all
	[ ".$(PWD)" != ".$(HERE)" ] || [ -f VERSION ] || \
	{ UP="`dirname "$(HERE)"`"; $(MAKE) -C"$$UP" it HERE="$$UP"; }

clean::
	$(RM) *.o *.d *~ .*~ */*~ $(CLEAN)
	-$(MAKE) -C tino clean HERE="$(HERE)"

distclean::	clean
	$(RM) "$(VERSIONFILE).h" $(PROGS) $(PROGS_EXE) $(DISTCLEAN)
	$(RM) core core.* .#*
	-$(MAKE) -C tino distclean HERE="$(HERE)"

# Special targets in presence of tinolib
# (subdirectory tino)

dist::	distclean
	-$(MAKE) -C tino dist HERE="$(HERE)" DEBUGS="$(DBG_CFLAGS)$(DBG_LDFLAGS)$(DBG_LDLIBS)"

tar::	distclean
	-$(MAKE) -C tino tar HERE="$(HERE)"

diff::
	-$(MAKE) -C tino diff HERE="$(HERE)"

# Automatically generated from $(SUBDIRS):

# automatically generated dependencies
$(PROG1).o:	$(COMMON)
$(PROG1):	$(PROG1).o $(OBJS) $(LIBS)

# compiler generated dependencies, remove if incorrect

# included: suid.d
$(PROG1).o:  suid.c linereader.h oops.h args.h memswap.h suid_version.h osx.h


#@MD5TINOIGN@ rules from: Makefile.tino

install::
	mkdir -p -m755 /etc/suid.conf.d
	chown 0:0  $(INSTALLPATH)/bin/suid /etc/suid.conf.d
	chmod 6555 $(INSTALLPATH)/bin/suid
	if [ -f '/etc/$(CONF)' ]; then install --backup=t --compare -m644 -o0 -g0 '$(CONF)' '/etc/$(CONF).dist'; else install -m644 -o0 -g0 '$(CONF)' '/etc/$(CONF)'; fi
	install --backup=t --compare -m644 -o0 -g0 -t/etc/suid.conf.d/ suid.conf.d.example/*.ex
# end
