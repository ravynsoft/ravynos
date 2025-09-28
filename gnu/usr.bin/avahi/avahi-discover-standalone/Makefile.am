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

interfaces = \
        avahi-discover.ui

# This cool debug trap works on i386/gcc only
AM_CFLAGS+='-DDEBUG_TRAP=__asm__("int $$3")'

if HAVE_GTK2OR3
if HAVE_GLIB
bin_PROGRAMS = \
	avahi-discover-standalone

avahi_discover_standalone_SOURCES = \
	main.c

avahi_discover_standalone_CFLAGS = \
	$(AM_CFLAGS) \
	-DAVAHI_INTERFACES_DIR=\"$(interfacesdir)\"

avahi_discover_standalone_LDADD = \
	$(AM_LDADD) \
	../avahi-common/libavahi-common.la \
	../avahi-glib/libavahi-glib.la \
	../avahi-core/libavahi-core.la

if HAVE_GTK3
avahi_discover_standalone_CFLAGS += \
	$(GLIB30_CFLAGS) $(GTK30_CFLAGS)
avahi_discover_standalone_LDADD += \
	$(GLIB30_LIBS) $(GTK30_LIBS)
else
avahi_discover_standalone_CFLAGS += \
	$(GLIB20_CFLAGS) $(GTK20_CFLAGS)
avahi_discover_standalone_LDADD += \
	$(GLIB20_LIBS) $(GTK20_LIBS)
endif

interfaces_DATA = $(interfaces)

endif  # HAVE_GLIB
else

if HAVE_PYTHON_DBUS

interfaces_DATA =

endif
endif

EXTRA_DIST = \
	$(interfaces)
