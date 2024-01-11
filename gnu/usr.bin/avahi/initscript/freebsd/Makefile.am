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

initddir = $(sysconfdir)/rc.d

EXTRA_DIST = \
	avahi-daemon.sh.in \
	avahi-dnsconfd.sh.in

initd_SCRIPTS = \
	avahi-daemon.sh \
	avahi-dnsconfd.sh

CLEANFILES = \
	avahi-daemon.sh \
	avahi-dnsconfd.sh

avahi-daemon.sh: avahi-daemon.sh.in
	sed -e 's,@sbindir\@,$(sbindir),g' $< > $@
	chmod +x $@

avahi-dnsconfd.sh: avahi-dnsconfd.sh.in
	sed \
	-e 's,@sbindir\@,$(sbindir),g' \
	-e 's,@sysconfdir\@,$(sysconfdir),g' \
	$< > $@
	chmod +x $@
