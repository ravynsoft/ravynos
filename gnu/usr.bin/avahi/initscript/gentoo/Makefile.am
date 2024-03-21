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

if !HAVE_DBUS
NO_DBUS_DEPENDENCY=/need dbus/d
endif

initddir = $(sysconfdir)/init.d

EXTRA_DIST = \
	avahi-daemon.in \
	avahi-dnsconfd.in

initd_SCRIPTS = \
	avahi-daemon \
	avahi-dnsconfd

CLEANFILES = \
	avahi-daemon \
	avahi-dnsconfd

avahi-daemon: avahi-daemon.in

	sed -e 's,@sbindir\@,$(sbindir),g; $(NO_DBUS_DEPENDENCY)' $< > $@
	chmod +x $@

avahi-dnsconfd: avahi-dnsconfd.in
	sed -e 's,@sbindir\@,$(sbindir),g' $< > $@
	chmod +x $@
