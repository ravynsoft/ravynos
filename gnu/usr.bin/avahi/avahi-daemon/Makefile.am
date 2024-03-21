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

if HAVE_LIBDAEMON
if HAVE_XML

pkgsysconfdir=$(sysconfdir)/avahi
servicedir=$(pkgsysconfdir)/services
introspectiondir=$(datadir)/dbus-1/interfaces
dbussystemservicesdir=$(datadir)/dbus-1/system-services

AM_CFLAGS+= \
	-DAVAHI_DAEMON_RUNTIME_DIR=\"$(avahi_runtime_dir)/avahi-daemon/\" \
	-DAVAHI_SOCKET=\"$(avahi_socket)\" \
	-DAVAHI_SERVICE_DIR=\"$(servicedir)\" \
	-DAVAHI_CONFIG_FILE=\"$(pkgsysconfdir)/avahi-daemon.conf\" \
	-DAVAHI_HOSTS_FILE=\"$(pkgsysconfdir)/hosts\" \
	-DAVAHI_DBUS_INTROSPECTION_DIR=\"$(introspectiondir)\" \
	-DAVAHI_CONFIG_DIR=\"$(pkgsysconfdir)\"

sbin_PROGRAMS = \
	avahi-daemon

if ENABLE_TESTS
noinst_PROGRAMS = \
	ini-file-parser-test
endif

avahi_daemon_SOURCES = \
	main.c main.h \
	simple-protocol.c simple-protocol.h \
	static-services.c static-services.h \
	static-hosts.c static-hosts.h \
	ini-file-parser.c ini-file-parser.h \
	setproctitle.c setproctitle.h \
	sd-daemon.h sd-daemon.c \
	../avahi-client/check-nss.c

avahi_daemon_CFLAGS = $(AM_CFLAGS) $(LIBDAEMON_CFLAGS) $(XML_CFLAGS)
avahi_daemon_LDADD = $(AM_LDADD) ../avahi-common/libavahi-common.la ../avahi-core/libavahi-core.la $(LIBDAEMON_LIBS) $(XML_LIBS)

ini_file_parser_test_SOURCES = \
	ini-file-parser.c ini-file-parser.h \
	ini-file-parser-test.c

ini_file_parser_test_CFLAGS = $(AM_CFLAGS)
ini_file_parser_test_LDADD = $(AM_LDADD) ../avahi-common/libavahi-common.la ../avahi-core/libavahi-core.la

pkgsysconf_DATA = \
	avahi-daemon.conf \
	hosts

dist_service_DATA = \
	ssh.service \
	sftp-ssh.service

dist_pkgdata_DATA = \
	avahi-service.dtd

%.service: %.service.in
	$(AM_V_GEN)sed -e 's,@sbindir\@,$(sbindir),g' $< > $@

%.socket: %.socket.in
	$(AM_V_GEN)sed -e 's,@sbindir\@,$(sbindir),g' \
		-e 's,@avahi_runtime_dir\@,$(avahi_runtime_dir),g' $< > $@

if HAVE_SYSTEMD
systemdsystemunit_DATA = \
	avahi-daemon.service \
	avahi-daemon.socket

dist_dbussystemservices_DATA = \
	org.freedesktop.Avahi.service
endif

CLEANFILES = $(systemdsystemunit_DATA)

if ENABLE_CHROOT

avahi_daemon_SOURCES += \
	chroot.c chroot.h \
	caps.c caps.h

avahi_daemon_LDADD += -lcap

endif

if HAVE_DLOPEN
avahi_daemon_LDADD += -ldl
endif

if HAVE_DBUS

dbusservicedir=$(DBUS_SYS_DIR)

avahi_daemon_SOURCES += \
	dbus-protocol.c dbus-protocol.h \
	dbus-util.c dbus-util.h \
	dbus-internal.h \
	dbus-async-address-resolver.c \
	dbus-async-host-name-resolver.c \
	dbus-async-service-resolver.c \
	dbus-domain-browser.c \
	dbus-entry-group.c \
	dbus-service-browser.c \
	dbus-service-type-browser.c \
	dbus-sync-address-resolver.c \
	dbus-sync-host-name-resolver.c \
	dbus-sync-service-resolver.c \
	dbus-record-browser.c  \
	../avahi-common/dbus.c ../avahi-common/dbus.h \
	../avahi-common/dbus-watch-glue.c ../avahi-common/dbus-watch-glue.h

avahi_daemon_LDADD += \
	$(DBUS_LIBS)

avahi_daemon_CFLAGS += $(DBUS_CFLAGS) -DDBUS_SYSTEM_BUS_DEFAULT_ADDRESS=\"$(DBUS_SYSTEM_BUS_DEFAULT_ADDRESS)\"

dist_dbusservice_DATA = avahi-dbus.conf

dist_introspection_DATA = \
	org.freedesktop.Avahi.Server.xml \
	org.freedesktop.Avahi.EntryGroup.xml \
	org.freedesktop.Avahi.DomainBrowser.xml \
	org.freedesktop.Avahi.ServiceTypeBrowser.xml \
	org.freedesktop.Avahi.ServiceBrowser.xml \
	org.freedesktop.Avahi.ServiceResolver.xml \
	org.freedesktop.Avahi.AddressResolver.xml \
	org.freedesktop.Avahi.HostNameResolver.xml \
	org.freedesktop.Avahi.RecordBrowser.xml

endif
endif
endif

EXTRA_DIST = \
	avahi-daemon.conf \
	example.service \
	hosts \
	example.service \
	introspect.dtd \
	introspect.xsl \
	avahi-daemon.service.in \
	avahi-daemon.socket.in

xmllint:
	xmllint --noout --valid example.service
	for F in $(introspection_DATA) ; do \
		xmllint --noout --valid $$F ; \
	done

install-data-local:
	test -z "$(avahi_runtime_dir)" || $(MKDIR_P) "$(DESTDIR)$(avahi_runtime_dir)"

update-systemd:
	curl http://cgit.freedesktop.org/systemd/plain/src/sd-daemon.c > sd-daemon.c
	curl http://cgit.freedesktop.org/systemd/plain/src/sd-daemon.h > sd-daemon.h
