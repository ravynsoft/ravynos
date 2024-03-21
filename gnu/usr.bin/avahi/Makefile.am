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

ACLOCAL_AMFLAGS = -I common

include $(srcdir)/common/doxygen.mk

EXTRA_DIST = config.rpath  \
	autogen.sh \
	bootstrap.sh \
	LICENSE \
	$(DX_CONFIG) \
	docs/INSTALL \
	docs/TODO \
	docs/NEWS \
	docs/README \
	docs/DBUS-API \
	docs/AUTHORS \
	docs/HACKING \
	docs/API-CHANGES-0.6 \
	docs/COMPAT-LAYERS \
	docs/MALLOC \
	docs/overview.dia \
	docs/server-states.dia \
	docs/avahi-poll.dia \
	avahi-core.pc.in \
	avahi-client.pc.in \
	avahi-glib.pc.in \
	avahi-libevent.pc.in \
	avahi-gobject.pc.in \
	avahi-qt3.pc.in \
	avahi-qt4.pc.in \
	avahi-qt5.pc.in \
	avahi-sharp.pc.in \
	avahi-ui-sharp.pc.in \
	avahi-compat-libdns_sd.pc.in \
	avahi-compat-howl.pc.in \
	avahi-ui.pc.in \
	avahi-ui-gtk3.pc.in \
	doxygen_to_devhelp.xsl \
        common/introspection.m4

SUBDIRS = \
	common \
	avahi-common \
	avahi-core \
	avahi-qt \
	avahi-client \
	avahi-glib \
	avahi-gobject \
	avahi-discover-standalone \
	avahi-daemon \
	avahi-sharp \
	initscript \
	avahi-dnsconfd \
	avahi-utils \
	avahi-python \
	examples \
	man \
	tests \
	service-type-database \
	avahi-compat-libdns_sd \
	avahi-compat-howl \
	avahi-autoipd \
	avahi-ui \
	avahi-ui-sharp \
	avahi-libevent \
	po

DX_INPUT = \
	$(srcdir)/avahi-common/address.h \
	$(srcdir)/avahi-common/malloc.h \
	$(srcdir)/avahi-common/strlst.h \
	$(srcdir)/avahi-common/alternative.h \
	$(srcdir)/avahi-common/defs.h \
	$(srcdir)/avahi-common/error.h \
	$(srcdir)/avahi-common/domain.h \
	$(srcdir)/avahi-common/watch.h \
	$(srcdir)/avahi-common/simple-watch.h \
	$(srcdir)/avahi-common/thread-watch.h

DX_EXAMPLE_PATH = $(srcdir)/examples
DX_EXAMPLE_PATTERNS = *.c

if HAVE_QT3
DX_INPUT += \
	$(srcdir)/avahi-qt/qt-watch.h
else
if HAVE_QT4
DX_INPUT += \
	$(srcdir)/avahi-qt/qt-watch.h
endif
if HAVE_QT5
DX_INPUT += \
	$(srcdir)/avahi-qt/qt-watch.h
endif
endif

if HAVE_GLIB
DX_INPUT += \
	$(srcdir)/avahi-glib/glib-watch.h \
	$(srcdir)/avahi-glib/glib-malloc.h

if HAVE_GOBJECT
if HAVE_DBUS
DX_INPUT += \
	$(srcdir)/avahi-gobject/ga-client.h \
	$(srcdir)/avahi-gobject/ga-entry-group.h \
	$(srcdir)/avahi-gobject/ga-enums.h \
	$(srcdir)/avahi-gobject/ga-error.h \
	$(srcdir)/avahi-gobject/ga-record-browser.h \
	$(srcdir)/avahi-gobject/ga-service-browser.h \
	$(srcdir)/avahi-gobject/ga-service-resolver.h
endif
endif
endif

if HAVE_DBUS
DX_INPUT += \
	$(srcdir)/avahi-client/client.h \
	$(srcdir)/avahi-client/lookup.h \
	$(srcdir)/avahi-client/publish.h
endif

if HAVE_DBUS
if HAVE_GTK
DX_INPUT += \
	$(srcdir)/avahi-ui/avahi-ui.h
endif
endif

if ENABLE_CORE_DOCS
DX_INPUT += \
	$(srcdir)/avahi-core/core.h \
	$(srcdir)/avahi-core/lookup.h \
	$(srcdir)/avahi-core/publish.h \
	$(srcdir)/avahi-core/rr.h \
	$(srcdir)/avahi-core/log.h
endif

if HAVE_GTK
DX_INPUT += \
	$(srcdir)/avahi-ui/avahi-ui.h
endif

if HAVE_LIBEVENT
DX_INPUT += \
	$(srcdir)/avahi-libevent/libevent-watch.h
endif

pkgconfigdir = $(libdir)/pkgconfig

%.pc: %.pc.in
	$(AM_V_GEN)sed -e 's,@prefix\@,$(prefix),g' \
	    -e 's,@libdir\@,$(libdir),g' \
	    -e 's,@HOWL_COMPAT_VERSION\@,$(HOWL_COMPAT_VERSION),g' \
	    -e 's,@PACKAGE_VERSION\@,$(PACKAGE_VERSION),g' $< > $@

pkgconfig_DATA = avahi-core.pc
CLEANFILES = avahi-core.pc

if HAVE_DBUS
pkgconfig_DATA += avahi-client.pc
CLEANFILES += avahi-client.pc

if ENABLE_COMPAT_HOWL
pkgconfig_DATA += avahi-compat-howl.pc
CLEANFILES += avahi-compat-howl.pc
endif

if ENABLE_COMPAT_LIBDNS_SD
pkgconfig_DATA += avahi-compat-libdns_sd.pc
CLEANFILES += avahi-compat-libdns_sd.pc
endif

if HAVE_MONO
pkgconfig_DATA += avahi-sharp.pc avahi-ui-sharp.pc
CLEANFILES += avahi-sharp.pc avahi-ui-sharp.pc
endif

endif

if HAVE_GLIB
pkgconfig_DATA += avahi-glib.pc
CLEANFILES += avahi-glib.pc

if HAVE_GOBJECT
pkgconfig_DATA += avahi-gobject.pc
CLEANFILES += avahi-gobject.pc
endif
endif

if HAVE_GTK
if HAVE_DBUS
pkgconfig_DATA += avahi-ui.pc
CLEANFILES += avahi-ui.pc
endif
endif

if HAVE_GTK3
if HAVE_DBUS
pkgconfig_DATA += avahi-ui-gtk3.pc
CLEANFILES += avahi-ui-gtk3.pc
endif
endif

if HAVE_QT3
pkgconfig_DATA += avahi-qt3.pc
CLEANFILES += avahi-qt3.pc
endif

if HAVE_QT4
pkgconfig_DATA += avahi-qt4.pc
CLEANFILES += avahi-qt4.pc
endif

if HAVE_QT5
pkgconfig_DATA += avahi-qt5.pc
CLEANFILES += avahi-qt5.pc
endif

if HAVE_LIBEVENT
pkgconfig_DATA += avahi-libevent.pc
CLEANFILES += avahi-libevent.pc
endif

CLEANFILES += avahi.devhelp

avahi.devhelp: doxygen-run
	xsltproc -o $@ doxygen_to_devhelp.xsl doxygen/xml/index.xml

MOSTLYCLEANFILES = $(DX_CLEANFILES)

DISTCHECK_CONFIGURE_FLAGS = \
	--disable-monodoc \
	--enable-introspection \
	--with-systemdsystemunitdir=$$dc_install_base/$(systemdsystemunitdir)

homepage:
	$(MAKE) -C man
	scp avahi-daemon/*.xml avahi-daemon/introspect.dtd avahi-daemon/introspect.xsl\
	    man/*.xml man/xmltoman.dtd man/xmltoman.xsl \
	    tango:www/avahi.org/tree/download/
	scp avahi-$(PACKAGE_VERSION).tar.gz tango:www/avahi.org/tree/download/
	rm -rf doxygen
	$(MAKE) doxygen-run
	ssh tango rm -rf www/avahi.org/tree/download/doxygen
	scp -r doxygen/html tango:www/avahi.org/tree/download/doxygen
