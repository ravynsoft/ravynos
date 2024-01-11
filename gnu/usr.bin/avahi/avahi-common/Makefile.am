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

avahi_commonincludedir=$(includedir)/avahi-common

avahi_commoninclude_HEADERS = \
	strlst.h \
	address.h \
	alternative.h \
	domain.h \
	cdecl.h \
	defs.h \
	malloc.h \
	watch.h \
	timeval.h \
	simple-watch.h \
	thread-watch.h \
	gccmacro.h \
	error.h \
	llist.h \
	rlist.h

if ENABLE_TESTS
noinst_PROGRAMS = \
	strlst-test \
	domain-test \
	alternative-test \
	timeval-test \
	watch-test \
	watch-test-thread \
	utf8-test
endif

lib_LTLIBRARIES = \
	libavahi-common.la

libavahi_common_la_SOURCES = \
	malloc.c malloc.h \
	address.c address.h \
	alternative.c alternative.h \
	error.c error.h \
	strlst.c strlst.h \
	domain.c domain.h \
	timeval.c timeval.h \
	simple-watch.c simple-watch.h \
	thread-watch.c thread-watch.h \
	watch.h gccmacro.h \
	rlist.h rlist.c \
	utf8.c utf8.h \
	i18n.c i18n.h

libavahi_common_la_CFLAGS = $(AM_CFLAGS) $(PTHREAD_CFLAGS) -DAVAHI_LOCALEDIR=\"$(avahilocaledir)\"
libavahi_common_la_LIBADD = $(AM_LDADD) $(PTHREAD_CFLAGS) $(PTHREAD_LIBS) $(INTLLIBS)
libavahi_common_la_LDFLAGS = $(AM_LDFLAGS)  -version-info $(LIBAVAHI_COMMON_VERSION_INFO)

strlst_test_SOURCES = \
	strlst.c strlst.h \
	malloc.c malloc.h \
	strlst-test.c
strlst_test_CFLAGS = $(AM_CFLAGS)

alternative_test_SOURCES = \
	alternative.c alternative.h \
	malloc.c malloc.h \
	domain.c domain.h \
	address.c address.h \
	alternative-test.c \
	utf8.c utf8.h
alternative_test_CFLAGS = $(AM_CFLAGS)

domain_test_SOURCES = \
	domain.c domain.h \
	malloc.c malloc.h \
	address.c address.h \
	domain-test.c \
	utf8.c utf8.h
domain_test_CFLAGS = $(AM_CFLAGS)

watch_test_SOURCES = \
	timeval.c timeval.h \
	simple-watch.c simple-watch.h \
	watch.h \
	malloc.c malloc.h \
	watch-test.c
watch_test_CFLAGS = $(AM_CFLAGS) $(PTHREAD_CFLAGS)
watch_test_LDADD = $(AM_LDADD) $(PTHREAD_LIBS) $(PTHREAD_CFLAGS)

watch_test_thread_SOURCES = $(watch_test_SOURCES) thread-watch.c thread-watch.h
watch_test_thread_CFLAGS = $(watch_test_CFLAGS) -DUSE_THREAD
watch_test_thread_LDADD = $(watch_test_LDADD)

timeval_test_SOURCES = \
	timeval.c timeval.h \
	timeval-test.c
timeval_test_CFLAGS = $(AM_CFLAGS) $(PTHREAD_CFLAGS)
timeval_test_LDADD = $(AM_LDADD) $(PTHREAD_LIBS) $(PTHREAD_CFLAGS)

utf8_test_SOURCES = \
	utf8-test.c \
	utf8.c utf8.h
utf8_test_CFLAGS = $(AM_CFLAGS)
utf8_test_LDADD = $(AM_LDADD)

if HAVE_DBUS

noinst_HEADERS = \
	dbus.h \
	dbus-watch-glue.h

endif
