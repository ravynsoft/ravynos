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

if HAVE_LIBEVENT

avahilibeventincludedir=$(includedir)/avahi-libevent

avahilibeventinclude_HEADERS = \
	libevent-watch.h

lib_LTLIBRARIES = \
	libavahi-libevent.la

if ENABLE_TESTS
noinst_PROGRAMS = \
	libevent-watch-test
endif

libavahi_libevent_la_SOURCES = \
	libevent-watch.c libevent-watch.h

libavahi_libevent_la_CFLAGS = $(AM_CFLAGS) $(LIBEVENT_CFLAGS)
libavahi_libevent_la_LIBADD = $(AM_LDADD) ../avahi-common/libavahi-common.la $(LIBEVENT_LIBS)
libavahi_libevent_la_LDFLAGS = $(AM_LDFLAGS)  -version-info $(LIBAVAHI_LIBEVENT_VERSION_INFO)

libevent_watch_test_SOURCES = \
	libevent-watch.c libevent-watch.h \
	libevent-watch-test.c
libevent_watch_test_CFLAGS = $(AM_CFLAGS) $(LIBEVENT_CFLAGS)
libevent_watch_test_LDADD = $(AM_LDADD) ../avahi-common/libavahi-common.la $(LIBEVENT_LIBS)

endif
