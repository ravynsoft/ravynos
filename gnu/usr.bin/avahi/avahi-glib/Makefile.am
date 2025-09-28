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

if HAVE_GLIB

avahiglibincludedir=$(includedir)/avahi-glib

avahiglibinclude_HEADERS = \
	glib-watch.h \
	glib-malloc.h

lib_LTLIBRARIES = \
	libavahi-glib.la

if ENABLE_TESTS
noinst_PROGRAMS = \
	glib-watch-test
endif

libavahi_glib_la_SOURCES = \
	glib-watch.c glib-watch.h \
	glib-malloc.h glib-malloc.c

libavahi_glib_la_CFLAGS = $(AM_CFLAGS) $(GLIB20_CFLAGS)
libavahi_glib_la_LIBADD = $(AM_LDADD) ../avahi-common/libavahi-common.la $(GLIB20_LIBS)
libavahi_glib_la_LDFLAGS = $(AM_LDFLAGS)  -version-info $(LIBAVAHI_GLIB_VERSION_INFO)

glib_watch_test_SOURCES = \
	glib-watch.c glib-watch.h \
	glib-watch-test.c
glib_watch_test_CFLAGS = $(AM_CFLAGS) $(GLIB20_CFLAGS)
glib_watch_test_LDADD = $(AM_LDADD) ../avahi-common/libavahi-common.la $(GLIB20_LIBS)

endif
