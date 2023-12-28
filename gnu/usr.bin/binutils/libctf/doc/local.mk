## Process this file with automake to produce Makefile.in.
#
#   Copyright (C) 2019-2023 Free Software Foundation, Inc.
#
# This file is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING.  If not see
# <http://www.gnu.org/licenses/>.
#

if BUILD_INFO

info_TEXINFOS += %D%/ctf-spec.texi

AM_MAKEINFOFLAGS = --no-split

DISTCLEANFILES += texput.log
MAINTAINERCLEANFILES += %D%/ctf-spec.info

html-local: %D%/ctf-spec/index.html
%D%/ctf-spec/index.html: %D%/ctf-spec.texi %D%/$(am__dirstamp)
	$(AM_V_GEN)$(MAKEINFOHTML) $(AM_MAKEINFOHTMLFLAGS) $(MAKEINFOFLAGS) \
	  --split=node -I$(srcdir) --output %D%/ctf-spec $(srcdir)/%D%/ctf-spec.texi

else

# Workaround bug in automake: it can't handle conditionally building info pages
# since GNU projects normally include info pages in the source distributions.
%D%/ctf-spec.info:

endif
