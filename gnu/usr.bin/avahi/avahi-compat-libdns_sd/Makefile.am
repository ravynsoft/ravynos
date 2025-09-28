# This file is part of avahi.
#
# avahi is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# avahi is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
# License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with avahi; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA.

AM_CFLAGS=-I$(top_srcdir)

# This cool debug trap works on i386/gcc only
AM_CFLAGS+='-DDEBUG_TRAP=__asm__("int $$3")'

if HAVE_DBUS
if ENABLE_COMPAT_LIBDNS_SD

avahi_compat_libdns_sddir=$(includedir)/avahi-compat-libdns_sd

avahi_compat_libdns_sd_HEADERS = dns_sd.h

lib_LTLIBRARIES = libdns_sd.la

if ENABLE_TESTS
noinst_PROGRAMS = txt-test null-test
endif

libdns_sd_la_SOURCES = \
	dns_sd.h \
	compat.c \
	txt.c \
	unsupported.c \
	warn.c warn.h
libdns_sd_la_CFLAGS = $(AM_CFLAGS) $(PTHREAD_CFLAGS)
libdns_sd_la_LDFLAGS = $(AM_LDFLAGS)  -version-info $(LIBAVAHI_COMPAT_LIBDNS_SD_VERSION_INFO) $(PTHREAD_LIBS) $(PTHREAD_CFLAGS) ../avahi-common/libavahi-common.la ../avahi-client/libavahi-client.la

txt_test_SOURCES = \
	dns_sd.h \
	txt.c \
	txt-test.c \
	warn.c warn.h
txt_test_CFLAGS = $(AM_CFLAGS) $(PTHREAD_CFLAGS)
txt_test_LDADD = $(AM_LDADD) $(PTHREAD_LIBS) $(PTHREAD_CFLAGS) ../avahi-common/libavahi-common.la

null_test_SOURCES = \
	dns_sd.h \
	compat.c \
	null-test.c \
	warn.c warn.h
null_test_CFLAGS = $(AM_CFLAGS) $(PTHREAD_CFLAGS)
null_test_LDADD = $(AM_LDADD) $(PTHREAD_LIBS) $(PTHREAD_CFLAGS) ../avahi-common/libavahi-common.la ../avahi-client/libavahi-client.la

# You can test the compatibility layer by sticking in mDNSResponder's
# dns-sd.c source here, naming it "libdns_sd-test.c" and running "make
# libdns_sd-test" manually. We do not distribute that file due to
# licensing restrictions.

libdns_sd-test: libdns_sd-test.c libdns_sd.la
	$(LIBTOOL) --mode=compile $(CC) $(AM_CFLAGS) $(PTHREAD_CFLAGS) -o libdns_sd-test.o -c libdns_sd-test.c
	$(LIBTOOL) --mode=link $(CC) $(AM_CFLAGS) $(PTHREAD_CFLAGS) -o libdns_sd-test libdns_sd-test.o $(PTHREAD_LIBS) ../avahi-common/libavahi-common.la libdns_sd.la

CLEANFILES = libdns_sd-test.o libdns_sd-test

endif
endif
