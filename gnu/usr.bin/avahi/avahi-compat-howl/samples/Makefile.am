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

AM_CFLAGS=-I$(top_srcdir) -I$(top_srcdir)/avahi-compat-howl/include

# This cool debug trap works on i386/gcc only
AM_CFLAGS+='-DDEBUG_TRAP=__asm__("int $$3")'

if HAVE_DBUS
if ENABLE_COMPAT_HOWL

if ENABLE_TESTS
noinst_PROGRAMS = browse resolve publish query
endif

browse_SOURCES = \
	browse.c
browse_CFLAGS = $(AM_CFLAGS) $(PTHREAD_CFLAGS) -I $(srcdir)/include
browse_LDADD = $(AM_LDADD) ../libhowl.la

resolve_SOURCES = \
	resolve.c
resolve_CFLAGS = $(AM_CFLAGS) $(PTHREAD_CFLAGS) -I $(srcdir)/include
resolve_LDADD = $(AM_LDADD) ../libhowl.la

publish_SOURCES = \
	publish.c
publish_CFLAGS = $(AM_CFLAGS) $(PTHREAD_CFLAGS) -I $(srcdir)/include
publish_LDADD = $(AM_LDADD) ../libhowl.la

query_SOURCES = \
	query.c
query_CFLAGS = $(AM_CFLAGS) $(PTHREAD_CFLAGS) -I $(srcdir)/include
query_LDADD = $(AM_LDADD) ../libhowl.la

endif
endif
