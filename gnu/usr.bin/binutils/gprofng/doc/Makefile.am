## Process this file with automake to generate Makefile.in
#
#   Copyright (C) 2012-2023 Free Software Foundation, Inc.
#
# This file is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING3.  If not see
# <http://www.gnu.org/licenses/>.
#

AUTOMAKE_OPTIONS = info-in-builddir foreign no-texinfo.tex

PDFS  = gprofng.pdf
HTMLS = gprofng.html

# Options to extract the man page
MANCONF = -Dman

TEXI2POD = perl $(srcdir)/../../etc/texi2pod.pl $(AM_MAKEINFOFLAGS)
POD2MAN = pod2man --center="User Commands" \
	--release="binutils-$(VERSION)" --section=1

info_TEXINFOS       = gprofng_ug.texi
gprofng_ug_TEXINFOS = fdl.texi gp-macros.texi
TEXINFO_TEX         = .
MAKEINFOHTML        = $(MAKEINFO) --html --no-split

man_MANS = gprofng.1 gp-archive.1 gp-collect-app.1 gp-display-html.1 gp-display-src.1 gp-display-text.1

# Build the man page from the texinfo file
# The sed command removes the no-adjust Nroff command so that
# the man output looks standard.
$(man_MANS): $(srcdir)/gp-macros.texi
	$(AM_V_GEN)touch $@
	$(AM_V_at)-$(TEXI2POD) $(MANCONF) < $(srcdir)/`basename $@ .1`.texi > $@.pod
	$(AM_V_at)-($(POD2MAN) $@.pod | sed -e '/^.if n .na/d' > $@.tmp && \
	  mv -f $@.tmp $@) || (rm -f $@.tmp && exit 1)
	$(AM_V_at)rm -f $@.pod

gprofng.1: $(srcdir)/gprofng.texi
gp-archive.1: $(srcdir)/gp-archive.texi
gp-collect-app.1: $(srcdir)/gp-collect-app.texi
gp-display-html.1: $(srcdir)/gp-display-html.texi
gp-display-src.1: $(srcdir)/gp-display-src.texi
gp-display-text.1: $(srcdir)/gp-display-text.texi

MAINTAINERCLEANFILES = gprofng.info $(man_MANS)
EXTRA_DIST = $(man_MANS) version.texi

info: $(man_MANS)
