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

if ENABLE_TESTS
noinst_PROGRAMS = \
	core-publish-service \
	core-browse-services
endif

core_publish_service_SOURCES = core-publish-service.c
core_publish_service_CFLAGS = $(AM_CFLAGS)
core_publish_service_LDADD = $(AM_LDADD) ../avahi-core/libavahi-core.la ../avahi-common/libavahi-common.la

core_browse_services_SOURCES = core-browse-services.c
core_browse_services_CFLAGS = $(AM_CFLAGS)
core_browse_services_LDADD = $(AM_LDADD) ../avahi-core/libavahi-core.la ../avahi-common/libavahi-common.la

if HAVE_DBUS
if ENABLE_TESTS

noinst_PROGRAMS += \
	client-publish-service \
	client-browse-services
endif

client_publish_service_SOURCES = client-publish-service.c
client_publish_service_CFLAGS = $(AM_CFLAGS)
client_publish_service_LDADD = $(AM_LDADD) ../avahi-client/libavahi-client.la ../avahi-common/libavahi-common.la

client_browse_services_SOURCES = client-browse-services.c
client_browse_services_CFLAGS = $(AM_CFLAGS)
client_browse_services_LDADD = $(AM_LDADD) ../avahi-client/libavahi-client.la ../avahi-common/libavahi-common.la

if HAVE_GLIB

if ENABLE_TESTS
noinst_PROGRAMS += \
	glib-integration
endif

glib_integration_SOURCES = glib-integration.c
glib_integration_CFLAGS = $(AM_CFLAGS) $(GLIB20_CFLAGS)
glib_integration_LDADD = $(AM_LDADD) $(GLIB20_LIBS) ../avahi-client/libavahi-client.la ../avahi-common/libavahi-common.la ../avahi-glib/libavahi-glib.la

endif

endif
