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

desktopdir = $(datadir)/applications
desktop_DATA =
desktop_DATA_in = $(desktop_DATA_in_in:.in.in=.in)
desktop_DATA_in_in = bssh.desktop.in.in bvnc.desktop.in.in

EXTRA_DIST = $(desktop_DATA_in_in)

if HAVE_GTK2OR3
AM_CFLAGS += -DGNOMELOCALEDIR=\"$(datadir)/locale\"
if HAVE_DBUS
if HAVE_GLIB

avahiincludedir=$(includedir)/avahi-ui

avahiinclude_HEADERS = \
	avahi-ui.h

lib_LTLIBRARIES =

if HAVE_GTK
lib_LTLIBRARIES += \
	libavahi-ui.la
endif

if HAVE_GTK3
lib_LTLIBRARIES += \
	libavahi-ui-gtk3.la
endif

libavahi_ui_la_SOURCES = \
	avahi-ui.h avahi-ui.c
libavahi_ui_la_CFLAGS = $(AM_CFLAGS) $(GTK20_CFLAGS)
libavahi_ui_la_LIBADD = $(AM_LDADD) ../avahi-common/libavahi-common.la ../avahi-client/libavahi-client.la ../avahi-glib/libavahi-glib.la $(GTK20_LIBS)
libavahi_ui_la_LDFLAGS = $(AM_LDFLAGS)  -version-info $(LIBAVAHI_UI_VERSION_INFO)

libavahi_ui_gtk3_la_SOURCES = $(libavahi_ui_la_SOURCES)
libavahi_ui_gtk3_la_CFLAGS = $(AM_CFLAGS) $(GTK30_CFLAGS)
libavahi_ui_gtk3_la_LIBADD = $(AM_LDADD) ../avahi-common/libavahi-common.la ../avahi-client/libavahi-client.la ../avahi-glib/libavahi-glib.la $(GTK30_LIBS)
libavahi_ui_gtk3_la_LDFLAGS = $(AM_LDFLAGS)  -version-info $(LIBAVAHI_UI_VERSION_INFO)

if HAVE_GDBM
libavahi_ui_la_SOURCES += ../avahi-utils/stdb.h ../avahi-utils/stdb.c
libavahi_ui_la_CFLAGS += -DDATABASE_FILE=\"$(pkglibdatadir)/service-types.db\"
libavahi_ui_la_LIBADD += -lgdbm

libavahi_ui_gtk3_la_CFLAGS += -DDATABASE_FILE=\"$(pkglibdatadir)/service-types.db\"
libavahi_ui_gtk3_la_LIBADD += -lgdbm
endif

if HAVE_DBM
libavahi_ui_la_SOURCES += ../avahi-utils/stdb.h ../avahi-utils/stdb.c
libavahi_ui_la_CFLAGS += -DDATABASE_FILE=\"$(pkglibdatadir)/service-types.db\"

libavahi_ui_gtk3_la_CFLAGS += -DDATABASE_FILE=\"$(pkglibdatadir)/service-types.db\"
endif

bin_PROGRAMS = bssh
desktop_DATA += bssh.desktop bvnc.desktop

bssh_SOURCES = bssh.c

if HAVE_GTK3
bssh_CFLAGS = $(AM_CFLAGS) $(GTK30_CFLAGS)
bssh_LDADD = $(AM_LDADD) $(GTK30_LIBS) ../avahi-client/libavahi-client.la ../avahi-common/libavahi-common.la libavahi-ui-gtk3.la
else
bssh_CFLAGS = $(AM_CFLAGS) $(GTK20_CFLAGS)
bssh_LDADD = $(AM_LDADD) $(GTK20_LIBS) ../avahi-client/libavahi-client.la ../avahi-common/libavahi-common.la libavahi-ui.la
endif

install-exec-local:
	cd $(DESTDIR)/$(bindir) && \
		rm -f bvnc bshell && \
		$(LN_S) bssh bvnc && \
		$(LN_S) bssh bshell

bssh.desktop.in: bssh.desktop.in.in
	$(AM_V_GEN)sed -e 's,@bindir\@,$(bindir),g' $< > $@

bssh.desktop: bssh.desktop.in
	$(AM_V_GEN)$(MSGFMT) --desktop --template $< -d $(top_srcdir)/po -o $@

bvnc.desktop.in: bvnc.desktop.in.in
	$(AM_V_GEN)sed -e 's,@bindir\@,$(bindir),g' $< > $@

bvnc.desktop: bvnc.desktop.in
	$(AM_V_GEN)$(MSGFMT) --desktop --template $< -d $(top_srcdir)/po -o $@

endif # HAVE_GLIB
endif
endif

CLEANFILES = $(desktop_DATA) $(desktop_DATA_in)
