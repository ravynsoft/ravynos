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

pkglibdatadir=$(libdir)/avahi

if HAVE_DBUS

bin_PROGRAMS = avahi-browse avahi-resolve avahi-publish avahi-set-host-name

avahi_browse_SOURCES = avahi-browse.c sigint.c sigint.h
avahi_browse_CFLAGS = $(AM_CFLAGS)
avahi_browse_LDADD = $(AM_LDADD) ../avahi-client/libavahi-client.la ../avahi-common/libavahi-common.la

if HAVE_GDBM
avahi_browse_SOURCES += stdb.h stdb.c
avahi_browse_CFLAGS += -DDATABASE_FILE=\"$(pkglibdatadir)/service-types.db\"
avahi_browse_LDADD += -lgdbm
endif

if HAVE_DBM
avahi_browse_SOURCES += stdb.h stdb.c
avahi_browse_CFLAGS += -DDATABASE_FILE=\"$(pkglibdatadir)/service-types.db\"
endif

avahi_resolve_SOURCES = avahi-resolve.c sigint.c sigint.h
avahi_resolve_CFLAGS = $(AM_CFLAGS)
avahi_resolve_LDADD = $(AM_LDADD) ../avahi-client/libavahi-client.la ../avahi-common/libavahi-common.la

avahi_publish_SOURCES = avahi-publish.c sigint.c sigint.h
avahi_publish_CFLAGS = $(AM_CFLAGS)
avahi_publish_LDADD = $(AM_LDADD) ../avahi-client/libavahi-client.la ../avahi-common/libavahi-common.la

avahi_set_host_name_SOURCES = avahi-set-host-name.c sigint.c sigint.h
avahi_set_host_name_CFLAGS = $(AM_CFLAGS)
avahi_set_host_name_LDADD = $(AM_LDADD) ../avahi-client/libavahi-client.la ../avahi-common/libavahi-common.la

install-exec-local:
	$(MKDIR_P) $(DESTDIR)/$(bindir) && \
		cd $(DESTDIR)/$(bindir) && \
		rm -f avahi-resolve-host-name avahi-resolve-address avahi-browse-domains avahi-publish-address avahi-publish-service && \
		$(LN_S) avahi-resolve avahi-resolve-host-name && \
		$(LN_S) avahi-resolve avahi-resolve-address && \
		$(LN_S) avahi-browse avahi-browse-domains && \
		$(LN_S) avahi-publish avahi-publish-address && \
		$(LN_S) avahi-publish avahi-publish-service

endif
