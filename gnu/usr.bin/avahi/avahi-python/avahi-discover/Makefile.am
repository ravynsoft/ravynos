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

EXTRA_DIST = \
	avahi-discover.py \
	avahi-discover.desktop.in.in

if HAVE_PYTHON
if HAVE_PYTHON_DBUS
if HAVE_PYGOBJECT

pythonscripts =

desktopdir = $(datadir)/applications
desktop_DATA =

if HAVE_GDBM
pythonscripts += \
	avahi-discover
desktop_DATA += avahi-discover.desktop
endif

if HAVE_DBM
pythonscripts += \
	avahi-discover
desktop_DATA += avahi-discover.desktop
endif

avahi-discover.desktop.in: avahi-discover.desktop.in.in
	$(AM_V_GEN)sed -e 's,@bindir\@,$(bindir),g' $< > $@

avahi-discover.desktop: avahi-discover.desktop.in
	$(AM_V_GEN)$(MSGFMT) --desktop --template $< -d $(top_srcdir)/po -o $@

avahi-discover: avahi-discover.py
	$(AM_V_GEN)sed -e 's,@PYTHON\@,$(PYTHON),g' \
		-e 's,@GETTEXT_PACKAGE\@,"$(GETTEXT_PACKAGE)",g' \
		-e 's,@LOCALEDIR\@,"$(avahilocaledir)",g' \
		-e 's,@interfacesdir\@,$(interfacesdir),g' $< > $@ && \
	chmod +x $@

bin_SCRIPTS = $(pythonscripts)

CLEANFILES = $(pythonscripts) $(desktop_DATA) *.pyc *.pyo avahi-discover.desktop.in

endif
endif
endif
