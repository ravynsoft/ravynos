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

# What version of the manual you want; "all" includes everything
CONFIG=all

# Options to extract the man page from as.texinfo
MANCONF = -Dman

TEXI2POD = perl $(top_srcdir)/../etc/texi2pod.pl $(AM_MAKEINFOFLAGS)

POD2MAN = pod2man --center="GNU Development Tools" \
	 --release="binutils-$(VERSION)" --section=1

# List of man pages generated from binutils.texi
man_MANS = \
	%D%/addr2line.1 \
	%D%/ar.1 \
	%D%/dlltool.1 \
	%D%/nm.1 \
	%D%/objcopy.1 \
	%D%/objdump.1 \
	%D%/ranlib.1 \
	%D%/readelf.1 \
	%D%/size.1 \
	%D%/strings.1 \
	%D%/strip.1 \
	%D%/elfedit.1 \
	%D%/windres.1 \
	%D%/windmc.1 \
	%D%/$(DEMANGLER_NAME).1

info_TEXINFOS = %D%/binutils.texi
binutils_TEXI = $(srcdir)/%D%/binutils.texi

AM_MAKEINFOFLAGS = -I "$(srcdir)/%D%" -I "$(top_srcdir)/../libiberty" \
		   -I "$(top_srcdir)/../bfd/doc" -I ../bfd/doc \
		   --no-split
TEXI2DVI = texi2dvi -I "$(srcdir)/%D%" -I "$(top_srcdir)/../libiberty" \
		    -I "$(top_srcdir)/../bfd/doc" -I ../bfd/doc

# Man page generation from texinfo
%D%/addr2line.1:	$(binutils_TEXI) doc/$(am__dirstamp)
	$(AM_V_GEN)touch $@
	$(AM_V_at)-$(TEXI2POD) $(MANCONF) -Daddr2line < $(binutils_TEXI) > addr2line.pod
	$(AM_V_at)-($(POD2MAN) addr2line.pod | sed -e '/^.if n .na/d' > $@.T$$$$ && \
		mv -f $@.T$$$$ $@) || (rm -f $@.T$$$$ && exit 1)
	$(AM_V_at)rm -f addr2line.pod

%D%/ar.1:	$(binutils_TEXI) doc/$(am__dirstamp)
	$(AM_V_GEN)touch $@
	$(AM_V_at)-$(TEXI2POD) $(MANCONF) -Dar < $(binutils_TEXI) > ar.pod
	$(AM_V_at)-($(POD2MAN) ar.pod | sed -e '/^.if n .na/d' > $@.T$$$$ && \
		mv -f $@.T$$$$ $@) || (rm -f $@.T$$$$ && exit 1)
	$(AM_V_at)rm -f ar.pod

%D%/dlltool.1:	$(binutils_TEXI) doc/$(am__dirstamp)
	$(AM_V_GEN)touch $@
	$(AM_V_at)-$(TEXI2POD) $(MANCONF) -Ddlltool < $(binutils_TEXI) > dlltool.pod
	$(AM_V_at)-($(POD2MAN) dlltool.pod | sed -e '/^.if n .na/d' > $@.T$$$$ && \
		mv -f $@.T$$$$ $@) || (rm -f $@.T$$$$ && exit 1)
	$(AM_V_at)rm -f dlltool.pod

%D%/nm.1:	$(binutils_TEXI) doc/$(am__dirstamp)
	$(AM_V_GEN)touch $@
	$(AM_V_at)-$(TEXI2POD) $(MANCONF) -Dnm < $(binutils_TEXI) > nm.pod
	$(AM_V_at)-($(POD2MAN) nm.pod | sed -e '/^.if n .na/d' > $@.T$$$$ && \
		mv -f $@.T$$$$ $@) || (rm -f $@.T$$$$ && exit 1)
	$(AM_V_at)rm -f nm.pod

%D%/objcopy.1:	$(binutils_TEXI) doc/$(am__dirstamp)
	$(AM_V_GEN)touch $@
	$(AM_V_at)-$(TEXI2POD) $(MANCONF) -Dobjcopy < $(binutils_TEXI) > objcopy.pod
	$(AM_V_at)-($(POD2MAN) objcopy.pod | sed -e '/^.if n .na/d' > $@.T$$$$ && \
		mv -f $@.T$$$$ $@) || (rm -f $@.T$$$$ && exit 1)
	$(AM_V_at)rm -f objcopy.pod

%D%/objdump.1:	$(binutils_TEXI) doc/$(am__dirstamp)
	$(AM_V_GEN)touch $@
	$(AM_V_at)-$(TEXI2POD) $(MANCONF) -Dobjdump < $(binutils_TEXI) > objdump.pod
	$(AM_V_at)-($(POD2MAN) objdump.pod | sed -e '/^.if n .na/d' > $@.T$$$$ && \
		mv -f $@.T$$$$ $@) || (rm -f $@.T$$$$ && exit 1)
	$(AM_V_at)rm -f objdump.pod

%D%/ranlib.1:	$(binutils_TEXI) doc/$(am__dirstamp)
	$(AM_V_GEN)touch $@
	$(AM_V_at)-$(TEXI2POD) $(MANCONF) -Dranlib < $(binutils_TEXI) > ranlib.pod
	$(AM_V_at)-($(POD2MAN) ranlib.pod | sed -e '/^.if n .na/d' > $@.T$$$$ && \
		mv -f $@.T$$$$ $@) || (rm -f $@.T$$$$ && exit 1)
	$(AM_V_at)rm -f ranlib.pod

%D%/readelf.1:	$(binutils_TEXI) doc/$(am__dirstamp)
	$(AM_V_GEN)touch $@
	$(AM_V_at)-$(TEXI2POD) $(MANCONF) -Dreadelf < $(binutils_TEXI) > readelf.pod
	$(AM_V_at)-($(POD2MAN) readelf.pod | sed -e '/^.if n .na/d' > $@.T$$$$ && \
		mv -f $@.T$$$$ $@) || (rm -f $@.T$$$$ && exit 1)
	$(AM_V_at)rm -f readelf.pod

%D%/size.1:	$(binutils_TEXI) doc/$(am__dirstamp)
	$(AM_V_GEN)touch $@
	$(AM_V_at)-$(TEXI2POD) $(MANCONF) -Dsize < $(binutils_TEXI) > size.pod
	$(AM_V_at)-($(POD2MAN) size.pod | sed -e '/^.if n .na/d' > $@.T$$$$ && \
		mv -f $@.T$$$$ $@) || (rm -f $@.T$$$$ && exit 1)
	$(AM_V_at)rm -f size.pod

%D%/strings.1:	$(binutils_TEXI) doc/$(am__dirstamp)
	$(AM_V_GEN)touch $@
	$(AM_V_at)-$(TEXI2POD) $(MANCONF) -Dstrings < $(binutils_TEXI) > strings.pod
	$(AM_V_at)-($(POD2MAN) strings.pod | sed -e '/^.if n .na/d' > $@.T$$$$ && \
		mv -f $@.T$$$$ $@) || (rm -f $@.T$$$$ && exit 1)
	$(AM_V_at)rm -f strings.pod

%D%/strip.1:	$(binutils_TEXI) doc/$(am__dirstamp)
	$(AM_V_GEN)touch $@
	$(AM_V_at)-$(TEXI2POD) $(MANCONF) -Dstrip < $(binutils_TEXI) > strip.pod
	$(AM_V_at)-($(POD2MAN) strip.pod | sed -e '/^.if n .na/d' > $@.T$$$$ && \
		mv -f $@.T$$$$ $@) || (rm -f $@.T$$$$ && exit 1)
	$(AM_V_at)rm -f strip.pod

%D%/elfedit.1:	$(binutils_TEXI) doc/$(am__dirstamp)
	$(AM_V_GEN)touch $@
	$(AM_V_at)-$(TEXI2POD) $(MANCONF) -Delfedit < $(binutils_TEXI) > elfedit.pod
	$(AM_V_at)-($(POD2MAN) elfedit.pod | sed -e '/^.if n .na/d' > $@.T$$$$ && \
		mv -f $@.T$$$$ $@) || (rm -f $@.T$$$$ && exit 1)
	$(AM_V_at)rm -f elfedit.pod

%D%/windres.1:	$(binutils_TEXI) doc/$(am__dirstamp)
	$(AM_V_GEN)touch $@
	$(AM_V_at)-$(TEXI2POD) $(MANCONF) -Dwindres < $(binutils_TEXI) > windres.pod
	$(AM_V_at)-($(POD2MAN) windres.pod | sed -e '/^.if n .na/d' > $@.T$$$$ && \
		mv -f $@.T$$$$ $@) || (rm -f $@.T$$$$ && exit 1)
	$(AM_V_at)rm -f windres.pod

%D%/windmc.1:	$(binutils_TEXI) doc/$(am__dirstamp)
	$(AM_V_GEN)touch $@
	$(AM_V_at)-$(TEXI2POD) $(MANCONF) -Dwindmc < $(binutils_TEXI) > windmc.pod
	$(AM_V_at)-($(POD2MAN) windmc.pod | sed -e '/^.if n .na/d' > $@.T$$$$ && \
		mv -f $@.T$$$$ $@) || (rm -f $@.T$$$$ && exit 1)
	$(AM_V_at)rm -f windmc.pod

%D%/cxxfilt.man:	$(binutils_TEXI) doc/$(am__dirstamp)
	$(AM_V_GEN)touch $@
	$(AM_V_at)-$(TEXI2POD) $(MANCONF) -Dcxxfilt < $(binutils_TEXI) > $(DEMANGLER_NAME).pod
	$(AM_V_at)-($(POD2MAN) $(DEMANGLER_NAME).pod | sed -e '/^.if n .na/d' > $@.T$$$$ && \
		mv -f $@.T$$$$ $@) || (rm -f $@.T$$$$ && exit 1)
	$(AM_V_at)rm -f $(DEMANGLER_NAME).pod

MAINTAINERCLEANFILES += $(man_MANS) %D%/binutils.info %D%/cxxfilt.man

%D%/$(DEMANGLER_NAME).1: %D%/cxxfilt.man Makefile doc/$(am__dirstamp)
	$(AM_V_GEN)if test -f %D%/cxxfilt.man; then \
	  man=%D%/cxxfilt.man; \
	else \
	  man=$(srcdir)/%D%/cxxfilt.man; \
	fi; \
	sed -e 's/cxxfilt/$(DEMANGLER_NAME)/' < $$man \
		> %D%/$(DEMANGLER_NAME).1

html-local: %D%/binutils/index.html
%D%/binutils/index.html: %D%/binutils.texi $(binutils_TEXINFOS)
	$(AM_V_GEN)$(MAKEINFOHTML) $(AM_MAKEINFOHTMLFLAGS) $(MAKEINFOFLAGS) \
	  --split=node -I$(srcdir) $(srcdir)/%D%/binutils.texi

# Maintenance

# We need it for the taz target in ../Makefile.in.
info-local: $(MANS)
