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

avahiincludedir=$(includedir)/avahi-core

avahiinclude_HEADERS = \
	core.h \
	log.h \
	rr.h \
	publish.h \
	lookup.h

lib_LTLIBRARIES = \
	libavahi-core.la

if ENABLE_TESTS
noinst_PROGRAMS = \
	prioq-test \
	avahi-test \
	conformance-test \
	avahi-reflector \
	dns-test \
	dns-spin-test \
	timeeventq-test \
	hashmap-test \
	querier-test \
	update-test

TESTS = \
	dns-spin-test \
	dns-test \
	hashmap-test
endif

libavahi_core_la_SOURCES = \
	timeeventq.c timeeventq.h\
	iface.c iface.h \
	server.c internal.h entry.c \
	prioq.c prioq.h \
	cache.c cache.h \
	socket.c socket.h \
	response-sched.c response-sched.h \
	query-sched.c query-sched.h \
	probe-sched.c probe-sched.h \
	announce.c announce.h \
	browse.c browse.h \
	rrlist.c rrlist.h \
	resolve-host-name.c \
	resolve-address.c \
	browse-domain.c \
	browse-service-type.c \
	browse-service.c \
	resolve-service.c \
	dns.c dns.h \
	rr.c rr.h rr-util.h \
	core.h lookup.h publish.h \
	log.c log.h \
	browse-dns-server.c \
	fdutil.h fdutil.c \
	util.c util.h \
	hashmap.c hashmap.h \
	wide-area.c wide-area.h \
	multicast-lookup.c multicast-lookup.h \
	querier.c querier.h \
	addr-util.h addr-util.c \
	domain-util.h domain-util.c \
	dns-srv-rr.h

if HAVE_NETLINK
libavahi_core_la_SOURCES += \
       iface-linux.c iface-linux.h \
       netlink.c netlink.h
else
if HAVE_PF_ROUTE
libavahi_core_la_SOURCES += \
	iface-pfroute.c iface-pfroute.h
else
libavahi_core_la_SOURCES += \
	iface-none.c
endif
endif

libavahi_core_la_CFLAGS = $(AM_CFLAGS)
libavahi_core_la_LIBADD = $(AM_LDADD) ../avahi-common/libavahi-common.la
libavahi_core_la_LDFLAGS = $(AM_LDFLAGS)  -version-info $(LIBAVAHI_CORE_VERSION_INFO)

prioq_test_SOURCES = \
	prioq-test.c  \
	prioq.c prioq.h
prioq_test_CFLAGS = $(AM_CFLAGS)
prioq_test_LDADD = $(AM_LDADD) ../avahi-common/libavahi-common.la

avahi_test_SOURCES = \
	avahi-test.c
avahi_test_CFLAGS = $(AM_CFLAGS)
avahi_test_LDADD = $(AM_LDADD) ../avahi-common/libavahi-common.la libavahi-core.la

update_test_SOURCES = \
	update-test.c
update_test_CFLAGS = $(AM_CFLAGS)
update_test_LDADD = $(AM_LDADD) ../avahi-common/libavahi-common.la libavahi-core.la

querier_test_SOURCES = \
	querier-test.c
querier_test_CFLAGS = $(AM_CFLAGS)
querier_test_LDADD = $(AM_LDADD) ../avahi-common/libavahi-common.la libavahi-core.la

conformance_test_SOURCES = \
	conformance-test.c
conformance_test_CFLAGS = $(AM_CFLAGS)
conformance_test_LDADD = $(AM_LDADD) ../avahi-common/libavahi-common.la libavahi-core.la

avahi_reflector_SOURCES = \
	avahi-reflector.c
avahi_reflector_CFLAGS = $(AM_CFLAGS)
avahi_reflector_LDADD = $(AM_LDADD) ../avahi-common/libavahi-common.la libavahi-core.la

dns_test_SOURCES = \
	dns.c dns.h \
	dns-test.c \
	log.c log.h \
	util.c util.h \
	rr.c rr.h \
	hashmap.c hashmap.h \
	domain-util.c domain-util.h \
	addr-util.c addr-util.h
dns_test_CFLAGS = $(AM_CFLAGS)
dns_test_LDADD = $(AM_LDADD) ../avahi-common/libavahi-common.la

dns_spin_test_SOURCES = \
	dns-spin-test.c
dns_spin_test_CFLAGS = $(AM_CFLAGS)
dns_spin_test_LDADD = $(AM_LDADD) libavahi-core.la

timeeventq_test_SOURCES = \
	timeeventq-test.c \
	timeeventq.h timeeventq.c \
	prioq.h prioq.c \
	log.c log.h
timeeventq_test_CFLAGS = $(AM_CFLAGS)
timeeventq_test_LDADD = $(AM_LDADD) ../avahi-common/libavahi-common.la

hashmap_test_SOURCES = \
	hashmap-test.c \
	hashmap.h hashmap.c \
	util.h util.c
hashmap_test_CFLAGS = $(AM_CFLAGS)
hashmap_test_LDADD = $(AM_LDADD) ../avahi-common/libavahi-common.la

valgrind: avahi-test
	libtool --mode=execute valgrind ./avahi-test

gdb: avahi-test
	libtool --mode=execute gdb ./avahi-test
