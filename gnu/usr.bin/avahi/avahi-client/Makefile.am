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

avahi_clientincludedir=$(includedir)/avahi-client
avahi_clientinclude_HEADERS = client.h lookup.h publish.h

noinst_HEADERS = internal.h

if ENABLE_TESTS

noinst_PROGRAMS = \
	client-test \
	srv-test \
	xdg-config-test \
	rr-test \
	check-nss-test

endif

lib_LTLIBRARIES = libavahi-client.la

libavahi_client_la_SOURCES = \
	client.c client.h \
	entrygroup.c \
	browser.c \
	resolver.c \
	publish.h lookup.h \
	xdg-config.c xdg-config.h \
	check-nss.c \
	../avahi-common/dbus.c ../avahi-common/dbus.h \
	../avahi-common/dbus-watch-glue.c ../avahi-common/dbus-watch-glue.h

libavahi_client_la_CFLAGS = $(AM_CFLAGS) $(DBUS_CFLAGS) -DDBUS_SYSTEM_BUS_DEFAULT_ADDRESS=\"$(DBUS_SYSTEM_BUS_DEFAULT_ADDRESS)\"
libavahi_client_la_LIBADD = $(AM_LDADD) $(DBUS_LIBS) ../avahi-common/libavahi-common.la
libavahi_client_la_LDFLAGS = $(AM_LDFLAGS)  -version-info $(LIBAVAHI_CLIENT_VERSION_INFO)

client_test_SOURCES = client-test.c
client_test_CFLAGS = $(AM_CFLAGS)
client_test_LDADD = $(AM_LDADD) libavahi-client.la ../avahi-common/libavahi-common.la

srv_test_SOURCES = srv-test.c
srv_test_CFLAGS = $(AM_CFLAGS)
srv_test_LDADD = $(AM_LDADD) libavahi-client.la ../avahi-common/libavahi-common.la

rr_test_SOURCES = rr-test.c
rr_test_CFLAGS = $(AM_CFLAGS)
rr_test_LDADD = $(AM_LDADD) libavahi-client.la ../avahi-common/libavahi-common.la

xdg_config_test_SOURCES = xdg-config-test.c xdg-config.c xdg-config.h
xdg_config_test_CFLAGS = $(AM_CFLAGS)
xdg_config_test_LDADD = $(AM_LDADD)

check_nss_test_SOURCES = check-nss.c check-nss-test.c client.h
check_nss_test_CFLAGS = $(AM_CFLAGS)
check_nss_test_LDADD = $(AM_LDADD)

if HAVE_DLOPEN
check_nss_test_LDADD += -ldl
libavahi_client_la_LIBADD += -ldl
endif

endif
