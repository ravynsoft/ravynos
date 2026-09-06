prefix=/usr/local
bindir=$(prefix)/bin
includedir=$(prefix)/include
libdir=$(prefix)/lib
sysconfdir=$(prefix)/etc
datarootdir=$(prefix)/share
datadir=$(datarootdir)/gettext-tiny
acdir=$(datarootdir)/aclocal

ifeq ($(LIBINTL), MUSL)
	LIBSRC = libintl/libintl-musl.c
	HEADERS =
else ifeq ($(LIBINTL), NONE)
	LIBSRC =
	HEADERS =
else
	LIBSRC = libintl/libintl.c
	HEADERS = libintl.h
endif
PROGSRC = $(sort $(wildcard src/*.c))

PARSEROBJS = src/poparser.o src/poparser_sysdep.o src/StringEscape.o
PROGOBJS = $(PROGSRC:.c=.o)
LIBOBJS = $(LIBSRC:.c=.o)
OBJS = $(PROGOBJS) $(LIBOBJS)

ALL_INCLUDES = $(HEADERS)
ifneq ($(LIBINTL), NONE)
ALL_LIBS=libintl.a
endif
ALL_TOOLS=msgfmt msgmerge xgettext autopoint
ALL_M4S=$(sort $(wildcard m4/*.m4))
ALL_DATA=$(sort $(wildcard data/*))

CFLAGS  ?= -O0 -fPIC

AR      ?= $(CROSS_COMPILE)ar
RANLIB  ?= $(CROSS_COMPILE)ranlib
CC      ?= $(CROSS_COMPILE)cc

INSTALL ?= ./install.sh

-include config.mak

LDLIBS:=$(shell echo "int main(){}" | $(CC) $(CFLAGS) $(LDFLAGS) -liconv -x c - >/dev/null 2>&1 && printf %s -liconv)

BUILDCFLAGS=$(CFLAGS)

all: $(ALL_LIBS) $(ALL_TOOLS)

install: $(ALL_LIBS:lib%=$(DESTDIR)$(libdir)/lib%) $(ALL_INCLUDES:%=$(DESTDIR)$(includedir)/%) $(ALL_TOOLS:%=$(DESTDIR)$(bindir)/%) $(ALL_M4S:%=$(DESTDIR)$(datadir)/%) $(ALL_M4S:%=$(DESTDIR)$(acdir)/%) $(ALL_DATA:%=$(DESTDIR)$(datadir)/%)

clean:
	rm -f $(ALL_LIBS)
	rm -f $(OBJS)
	rm -f $(ALL_TOOLS)

%.o: %.c
	$(CC) $(BUILDCFLAGS) -c -o $@ $<

libintl.a: $(LIBOBJS)
	rm -f $@
	$(AR) rc $@ $(LIBOBJS)
	$(RANLIB) $@

msgmerge: $(OBJS)
	$(CC) -o $@ src/msgmerge.o $(PARSEROBJS) $(LDFLAGS) $(LDLIBS)

msgfmt: $(OBJS)
	$(CC) -o $@ src/msgfmt.o $(PARSEROBJS) $(LDFLAGS) $(LDLIBS)

xgettext:
	cp src/xgettext.sh ./xgettext

autopoint: src/autopoint.in
	cat $< | sed 's,@datadir@,$(datadir),' > $@

$(DESTDIR)$(libdir)/%.a: %.a
	$(INSTALL) -D -m 755 $< $@

$(DESTDIR)$(includedir)/%.h: include/%.h
	$(INSTALL) -D -m 644 $< $@

$(DESTDIR)$(bindir)/%: %
	$(INSTALL) -D -m 755 $< $@

$(DESTDIR)$(datadir)/%: %
	$(INSTALL) -D -m 644 $< $@

$(DESTDIR)$(acdir)/%: %
	$(INSTALL) -D -l ../$(subst $(datarootdir)/,,$(datadir))/$< $(patsubst %m4/,%,$(dir $@))/$(notdir $@)

.PHONY: all clean install
