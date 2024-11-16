#
#   DMI Decode
#   BIOS Decode
#   VPD Decode
#
#   Copyright (C) 2000-2002 Alan Cox <alan@redhat.com>
#   Copyright (C) 2002-2020 Jean Delvare <jdelvare@suse.de>
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#

CC     ?= gcc
# Base CFLAGS can be overridden by environment
CFLAGS ?= -O2
# When debugging, disable -O2 and enable -g
#CFLAGS ?= -g

CFLAGS += -W -Wall -Wshadow -Wstrict-prototypes -Wpointer-arith -Wcast-qual \
          -Wcast-align -Wwrite-strings -Wmissing-prototypes -Winline -Wundef

# Let lseek and mmap support 64-bit wide offsets
CFLAGS += -D_FILE_OFFSET_BITS=64

#CFLAGS += -DBIGENDIAN
#CFLAGS += -DALIGNMENT_WORKAROUND

# Pass linker flags here (can be set from environment too)
LDFLAGS ?=

DESTDIR =
prefix  = /usr
sbindir = $(prefix)/sbin
mandir  = $(prefix)/share/man
man8dir = $(mandir)/man8
docdir  = $(prefix)/share/doc/dmidecode
compdir = $(shell pkg-config --variable=completionsdir bash-completion 2>/dev/null || echo $(prefix)/etc/bash_completion.d)

INSTALL         := install
INSTALL_DATA    := $(INSTALL) -m 644
INSTALL_DIR     := $(INSTALL) -m 755 -d
INSTALL_PROGRAM := $(INSTALL) -m 755
RM              := rm -f

# BSD make provides $MACHINE, but GNU make doesn't
MACHINE ?= $(shell uname -m 2>/dev/null)

# These programs are only useful on x86
PROGRAMS-i386 := biosdecode ownership vpddecode
PROGRAMS-i486 := $(PROGRAMS-i386)
PROGRAMS-i586 := $(PROGRAMS-i386)
PROGRAMS-i686 := $(PROGRAMS-i386)
PROGRAMS-x86_64 := biosdecode ownership vpddecode
PROGRAMS-amd64 := $(PROGRAMS-x86_64)

PROGRAMS := dmidecode $(PROGRAMS-$(MACHINE))

all : $(PROGRAMS)

#
# Programs
#

dmidecode : dmidecode.o dmiopt.o dmioem.o dmioutput.o util.o
	$(CC) $(LDFLAGS) dmidecode.o dmiopt.o dmioem.o dmioutput.o util.o -o $@

biosdecode : biosdecode.o util.o
	$(CC) $(LDFLAGS) biosdecode.o util.o -o $@

ownership : ownership.o util.o
	$(CC) $(LDFLAGS) ownership.o util.o -o $@

vpddecode : vpddecode.o vpdopt.o util.o
	$(CC) $(LDFLAGS) vpddecode.o vpdopt.o util.o -o $@

#
# Objects
#

dmidecode.o : dmidecode.c version.h types.h util.h config.h dmidecode.h \
	      dmiopt.h dmioem.h dmioutput.h
	$(CC) $(CFLAGS) -c $< -o $@

dmiopt.o : dmiopt.c config.h types.h util.h dmidecode.h dmiopt.h
	$(CC) $(CFLAGS) -c $< -o $@

dmioem.o : dmioem.c types.h dmidecode.h dmioem.h dmioutput.h
	$(CC) $(CFLAGS) -c $< -o $@

dmioutput.o : dmioutput.c types.h dmioutput.h
	$(CC) $(CFLAGS) -c $< -o $@

biosdecode.o : biosdecode.c version.h types.h util.h config.h 
	$(CC) $(CFLAGS) -c $< -o $@

ownership.o : ownership.c version.h types.h util.h config.h
	$(CC) $(CFLAGS) -c $< -o $@

vpddecode.o : vpddecode.c version.h types.h util.h config.h vpdopt.h
	$(CC) $(CFLAGS) -c $< -o $@

vpdopt.o : vpdopt.c config.h util.h vpdopt.h
	$(CC) $(CFLAGS) -c $< -o $@

util.o : util.c types.h util.h config.h
	$(CC) $(CFLAGS) -c $< -o $@

#
# Commands
#

strip : $(PROGRAMS)
	strip $(PROGRAMS)

install : install-bin #install-man install-doc install-completion

uninstall : uninstall-bin #uninstall-man uninstall-doc uninstall-completion

install-bin : $(PROGRAMS)
	$(INSTALL_DIR) $(DESTDIR)$(sbindir)
	for program in $(PROGRAMS) ; do \
	$(INSTALL_PROGRAM) $$program $(DESTDIR)$(sbindir) ; done

uninstall-bin :
	for program in $(PROGRAMS) ; do \
	$(RM) $(DESTDIR)$(sbindir)/$$program ; done

install-man :
	$(INSTALL_DIR) $(DESTDIR)$(man8dir)
	for program in $(PROGRAMS) ; do \
	$(INSTALL_DATA) man/$$program.8 $(DESTDIR)$(man8dir) ; done

uninstall-man :
	for program in $(PROGRAMS) ; do \
	$(RM) $(DESTDIR)$(man8dir)/$$program.8 ; done

install-doc :
	$(INSTALL_DIR) $(DESTDIR)$(docdir)
	$(INSTALL_DATA) README $(DESTDIR)$(docdir)
	$(INSTALL_DATA) NEWS $(DESTDIR)$(docdir)
	$(INSTALL_DATA) AUTHORS $(DESTDIR)$(docdir)

uninstall-doc :
	$(RM) -r $(DESTDIR)$(docdir)

install-completion :
	if [ -d $(compdir) ] ; then \
	$(INSTALL_DIR) $(DESTDIR)$(compdir) ; \
	for program in $(PROGRAMS) ; do \
	$(INSTALL_DATA) completion/$$program.bash $(DESTDIR)$(compdir)/$$program ; done ; \
	fi

uninstall-completion :
	if [ -d $(DESTDIR)$(compdir) ]; then \
	for program in $(PROGRAMS) ; do \
	$(RM) $(DESTDIR)$(compdir)/$$program ; done ; \
	fi

clean :
	$(RM) *.o $(PROGRAMS) core

# Targets for etcupdate.sh
installetc:
buildetc:
installconfig:

# Targets for buildworld
includes:
distribute:
	$(MAKE) -C ${.CURDIR} DESTDIR=${OBJTOP}/release/dist/base/ install
