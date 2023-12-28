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

# Options to extract the man page from as.texi
MANCONF = -Dman

TEXI2POD = perl $(BASEDIR)/etc/texi2pod.pl $(AM_MAKEINFOFLAGS)

POD2MAN = pod2man --center="GNU Development Tools" \
	--release="binutils-$(VERSION)" --section=1

man_MANS = %D%/as.1

info_TEXINFOS = %D%/as.texi
%C%_as_TEXINFOS = %D%/asconfig.texi $(CPU_DOCS)

AM_MAKEINFOFLAGS = -I "$(srcdir)/%D%" -I %D% -I "$(srcdir)/../libiberty" \
		   -I "$(srcdir)/../bfd/doc" -I ../bfd/doc \
		   --no-split
TEXI2DVI = texi2dvi -I "$(srcdir)/%D%" -I %D% -I "$(srcdir)/../libiberty" \
		   -I "$(srcdir)/../bfd/doc" -I ../bfd/doc

%D%/asconfig.texi: %D%/$(CONFIG).texi %D%/$(am__dirstamp)
	$(AM_V_at)rm -f %D%/asconfig.texi
	$(AM_V_GEN)cp -p $(srcdir)/%D%/$(CONFIG).texi %D%/asconfig.texi
	$(AM_V_at)chmod u+w %D%/asconfig.texi

CPU_DOCS = \
	%D%/c-aarch64.texi \
	%D%/c-alpha.texi \
	%D%/c-arc.texi \
	%D%/c-arm.texi \
	%D%/c-avr.texi \
	%D%/c-bfin.texi \
	%D%/c-bpf.texi \
	%D%/c-cr16.texi \
	%D%/c-cris.texi \
	%D%/c-csky.texi \
	%D%/c-d10v.texi \
	%D%/c-epiphany.texi \
	%D%/c-h8300.texi \
	%D%/c-hppa.texi \
	%D%/c-i386.texi \
	%D%/c-ip2k.texi \
	%D%/c-lm32.texi \
	%D%/c-m32c.texi \
	%D%/c-m32r.texi \
	%D%/c-m68hc11.texi \
	%D%/c-m68k.texi \
	%D%/c-s12z.texi \
	%D%/c-metag.texi \
	%D%/c-microblaze.texi \
	%D%/c-mips.texi \
	%D%/c-mmix.texi \
	%D%/c-mt.texi \
	%D%/c-msp430.texi \
	%D%/c-nios2.texi \
	%D%/c-nds32.texi \
	%D%/c-ns32k.texi \
	%D%/c-or1k.texi \
	%D%/c-pdp11.texi \
	%D%/c-pj.texi \
	%D%/c-ppc.texi \
	%D%/c-pru.texi \
	%D%/c-rl78.texi \
	%D%/c-riscv.texi \
	%D%/c-rx.texi \
	%D%/c-s390.texi \
	%D%/c-score.texi \
	%D%/c-sh.texi \
	%D%/c-sparc.texi \
	%D%/c-tic54x.texi \
	%D%/c-tic6x.texi \
	%D%/c-tilegx.texi \
	%D%/c-tilepro.texi \
	%D%/c-v850.texi \
	%D%/c-vax.texi \
	%D%/c-visium.texi \
	%D%/c-xgate.texi \
	%D%/c-xstormy16.texi \
	%D%/c-xtensa.texi \
	%D%/c-z80.texi \
	%D%/c-z8k.texi

# This one isn't ready for prime time yet.  Not even a little bit.

noinst_TEXINFOS = %D%/internals.texi

MAINTAINERCLEANFILES += %D%/asconfig.texi

# Maintenance

# We need it for the taz target in ../Makefile.in.
info-local: $(MANS)

# Build the man page from the texinfo file
# The sed command removes the no-adjust Nroff command so that
# the man output looks standard.
%D%/as.1: $(srcdir)/%D%/as.texi %D%/asconfig.texi $(CPU_DOCS) %D%/$(am__dirstamp)
	$(AM_V_GEN)touch $@
	$(AM_V_at)-$(TEXI2POD) $(MANCONF) < $(srcdir)/%D%/as.texi > %D%/as.pod
	$(AM_V_at)-($(POD2MAN) %D%/as.pod | \
	        sed -e '/^.if n .na/d' > $@.T$$$$ && \
	        mv -f $@.T$$$$ $@) || \
	        (rm -f $@.T$$$$ && exit 1)
	$(AM_V_at)rm -f %D%/as.pod

html-local: %D%/as/index.html
%D%/as/index.html: %D%/as.texi $(%C%_as_TEXINFOS) %D%/$(am__dirstamp)
	$(AM_V_GEN)$(MAKEINFOHTML) $(AM_MAKEINFOHTMLFLAGS) $(MAKEINFOFLAGS) \
	  --split=node -I$(srcdir)/%D% -o %D%/as $(srcdir)/%D%/as.texi

MAINTAINERCLEANFILES += %D%/as.info
