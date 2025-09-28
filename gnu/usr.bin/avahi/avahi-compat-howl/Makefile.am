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

SUBDIRS = . samples

if HAVE_DBUS
if ENABLE_COMPAT_HOWL

avahi_compat_howldir=$(includedir)/avahi-compat-howl
avahi_compat_howl_rendezvousdir=$(avahi_compat_howldir)/rendezvous
avahi_compat_howl_corbydir=$(avahi_compat_howldir)/corby
avahi_compat_howl_discoverydir=$(avahi_compat_howldir)/discovery
avahi_compat_howl_saltdir=$(avahi_compat_howldir)/salt

avahi_compat_howl_HEADERS = \
	include/howl.h

avahi_compat_howl_rendezvous_HEADERS = \
	include/rendezvous/rendezvous.h \
	include/rendezvous/text_record.h

avahi_compat_howl_corby_HEADERS = \
	include/corby/message.h \
	include/corby/object.h \
	include/corby/orb.h \
	include/corby/corby.h \
	include/corby/channel.h \
	include/corby/buffer.h

avahi_compat_howl_discovery_HEADERS = \
	include/discovery/discovery.h \
	include/discovery/text_record.h

avahi_compat_howl_salt_HEADERS = \
	include/salt/socket.h \
	include/salt/address.h \
	include/salt/platform.h \
	include/salt/signal.h \
	include/salt/interface.h \
	include/salt/salt.h \
	include/salt/time.h \
	include/salt/debug.h

HOWLHEADERS = \
	$(avahi_compat_howl_HEADERS) \
	$(avahi_compat_howl_rendezvous_HEADERS) \
	$(avahi_compat_howl_corby_HEADERS) \
	$(avahi_compat_howl_discovery_HEADERS) \
	$(avahi_compat_howl_salt_HEADERS)

lib_LTLIBRARIES = libhowl.la

if ENABLE_TESTS
noinst_PROGRAMS = address-test text-test browse-domain-test
endif

libhowl_la_SOURCES = \
	$(HOWLHEADERS) \
	warn.c warn.h \
	unsupported.c \
	address.c \
	text.c \
	compat.c
libhowl_la_CFLAGS = $(AM_CFLAGS) $(PTHREAD_CFLAGS) -I $(srcdir)/include
libhowl_la_LDFLAGS = $(AM_LDFLAGS)  -version-info $(LIBAVAHI_COMPAT_HOWL_VERSION_INFO) $(PTHREAD_LIBS) $(PTHREAD_CFLAGS) ../avahi-common/libavahi-common.la ../avahi-client/libavahi-client.la

address_test_SOURCES = \
	$(HOWLHEADERS) \
	address.c \
	address-test.c \
	warn.c warn.h
address_test_CFLAGS = $(AM_CFLAGS) $(PTHREAD_CFLAGS) -I $(srcdir)/include
address_test_LDADD = $(AM_LDADD) $(PTHREAD_LIBS) $(PTHREAD_CFLAGS) ../avahi-common/libavahi-common.la

text_test_SOURCES = \
	$(HOWLHEADERS) \
	text.c \
	text-test.c \
	warn.c warn.h
text_test_CFLAGS = $(AM_CFLAGS)  $(PTHREAD_CFLAGS) -I $(srcdir)/include
text_test_LDADD = $(AM_LDADD) $(PTHREAD_LIBS) $(PTHREAD_CFLAGS) ../avahi-common/libavahi-common.la

browse_domain_test_SOURCES = \
	$(HOWLHEADERS) \
	browse-domain-test.c
browse_domain_test_CFLAGS = $(AM_CFLAGS) $(PTHREAD_CFLAGS) -I $(srcdir)/include
browse_domain_test_LDADD = $(AM_LDADD) $(PTHREAD_LIBS) $(PTHREAD_CFLAGS) libhowl.la

endif
endif
