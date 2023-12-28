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

AUTOMAKE_OPTIONS = foreign no-dist
ACLOCAL_AMFLAGS = -I .. -I ../config -I ../bfd

# Build '.' first so all generated files exist.
SUBDIRS = . po

INCDIR = $(srcdir)/../include
BFDDIR = $(srcdir)/../bfd

WARN_CFLAGS = @WARN_CFLAGS@
NO_WERROR = @NO_WERROR@
AM_CFLAGS = $(WARN_CFLAGS)

COMPILE_FOR_BUILD = $(CC_FOR_BUILD) $(INCLUDES) $(AM_CPPFLAGS) \
	$(CFLAGS_FOR_BUILD)
LINK_FOR_BUILD = $(CC_FOR_BUILD) $(CFLAGS_FOR_BUILD) \
	$(LDFLAGS_FOR_BUILD) -o $@

libopcodes_la_LDFLAGS =
if INSTALL_LIBBFD
bfdlibdir = @bfdlibdir@
bfdincludedir = @bfdincludedir@
bfdlib_LTLIBRARIES = libopcodes.la
bfdinclude_DATA = $(INCDIR)/dis-asm.h
else
# Empty these so that the respective installation directories will not be created.
bfdlibdir =
bfdincludedir =
rpath_bfdlibdir = @bfdlibdir@
noinst_LTLIBRARIES = libopcodes.la
libopcodes_la_LDFLAGS += -rpath $(rpath_bfdlibdir)
endif

# This is where bfd.h lives.
BFD_H = ../bfd/bfd.h

BUILD_LIBS = @BUILD_LIBS@
BUILD_LIB_DEPS = @BUILD_LIB_DEPS@

# Header files.
HFILES = \
	aarch64-asm.h aarch64-dis.h aarch64-opc.h aarch64-tbl.h \
	bpf-desc.h bpf-opc.h \
	cris-desc.h cris-opc.h \
	epiphany-desc.h epiphany-opc.h \
	fr30-desc.h fr30-opc.h \
	frv-desc.h frv-opc.h \
	i386-opc.h \
	ia64-asmtab.h \
	ia64-opc.h \
	ip2k-desc.h ip2k-opc.h \
	iq2000-desc.h iq2000-opc.h \
	lm32-desc.h \
	lm32-opc.h \
	m32c-desc.h m32c-opc.h \
	m32r-desc.h m32r-opc.h \
	mcore-opc.h \
	mep-desc.h mep-opc.h \
	microblaze-opc.h \
	mt-desc.h mt-opc.h \
	score-opc.h \
	sh-opc.h \
	sysdep.h \
	xstormy16-desc.h xstormy16-opc.h \
	z8k-opc.h

# C source files that correspond to .o's ending up in libopcodes
# for all machines.
TARGET64_LIBOPCODES_CFILES = \
	aarch64-asm.c \
	aarch64-asm-2.c \
	aarch64-dis.c \
	aarch64-dis-2.c \
	aarch64-opc.c \
	aarch64-opc-2.c \
	alpha-dis.c \
	alpha-opc.c \
	bpf-asm.c \
	bpf-desc.c \
	bpf-dis.c \
	bpf-ibld.c \
	bpf-opc.c \
	ia64-dis.c \
	ia64-opc.c \
	loongarch-opc.c \
	loongarch-dis.c \
	loongarch-coder.c \
	mips-dis.c \
	mips-opc.c \
	mips16-opc.c \
	micromips-opc.c \
	mmix-dis.c \
	mmix-opc.c \
	nfp-dis.c \
	riscv-dis.c \
	riscv-opc.c \
	tilegx-dis.c \
	tilegx-opc.c

TARGET32_LIBOPCODES_CFILES = \
	arc-dis.c \
	arc-ext.c \
	arc-opc.c \
	arm-dis.c \
	avr-dis.c \
	bfin-dis.c \
	cgen-asm.c \
	cgen-bitset.c \
	cgen-dis.c \
	cgen-opc.c \
	cr16-dis.c \
	cr16-opc.c \
	cris-desc.c \
	cris-dis.c \
	cris-opc.c \
	crx-dis.c \
	crx-opc.c \
	csky-dis.c \
	d10v-dis.c \
	d10v-opc.c \
	d30v-dis.c \
	d30v-opc.c \
	dlx-dis.c \
	epiphany-asm.c \
	epiphany-desc.c \
	epiphany-dis.c \
	epiphany-ibld.c \
	epiphany-opc.c \
	fr30-asm.c \
	fr30-desc.c \
	fr30-dis.c \
	fr30-ibld.c \
	fr30-opc.c \
	frv-asm.c \
	frv-desc.c \
	frv-dis.c \
	frv-ibld.c \
	frv-opc.c \
	ft32-dis.c \
	ft32-opc.c \
	h8300-dis.c \
	hppa-dis.c \
	i386-dis.c \
	ip2k-asm.c \
	ip2k-desc.c \
	ip2k-dis.c \
	ip2k-ibld.c \
	ip2k-opc.c \
	iq2000-asm.c \
	iq2000-desc.c \
	iq2000-dis.c \
	iq2000-ibld.c \
	iq2000-opc.c \
	lm32-asm.c \
	lm32-desc.c \
	lm32-dis.c \
	lm32-ibld.c \
	lm32-opc.c \
	lm32-opinst.c \
	m10200-dis.c \
	m10200-opc.c \
	m10300-dis.c \
	m10300-opc.c \
	m32c-asm.c \
	m32c-desc.c \
	m32c-dis.c \
	m32c-ibld.c \
	m32c-opc.c \
	m32r-asm.c \
	m32r-desc.c \
	m32r-dis.c \
	m32r-ibld.c \
	m32r-opc.c \
	m32r-opinst.c \
	m68hc11-dis.c \
	m68hc11-opc.c \
	m68k-dis.c \
	m68k-opc.c \
	s12z-dis.c \
	s12z-opc.c \
	mcore-dis.c \
	mep-asm.c \
	mep-desc.c \
	mep-dis.c \
	mep-ibld.c \
	mep-opc.c \
	metag-dis.c \
	microblaze-dis.c \
	moxie-dis.c \
	moxie-opc.c \
	msp430-decode.c \
	msp430-dis.c \
	mt-asm.c \
	mt-desc.c \
	mt-dis.c \
	mt-ibld.c \
	mt-opc.c \
	nds32-asm.c \
	nds32-dis.c \
	nios2-dis.c \
	nios2-opc.c \
	ns32k-dis.c \
	or1k-asm.c \
	or1k-desc.c \
	or1k-dis.c \
	or1k-ibld.c \
	or1k-opc.c \
	pdp11-dis.c \
	pdp11-opc.c \
	pj-dis.c \
	pj-opc.c \
	ppc-dis.c \
	ppc-opc.c \
	pru-dis.c \
	pru-opc.c \
	rl78-decode.c \
	rl78-dis.c \
	rx-decode.c \
	rx-dis.c \
	s390-dis.c \
	s390-opc.c \
	score-dis.c \
	score7-dis.c \
	sh-dis.c \
	sparc-dis.c \
	sparc-opc.c \
	spu-dis.c \
	spu-opc.c \
	tic30-dis.c \
	tic4x-dis.c \
	tic54x-dis.c \
	tic54x-opc.c \
	tic6x-dis.c \
	tilepro-dis.c \
	tilepro-opc.c \
	v850-dis.c \
	v850-opc.c \
	vax-dis.c \
	visium-dis.c \
	visium-opc.c \
	wasm32-dis.c \
	xstormy16-asm.c \
	xstormy16-desc.c \
	xstormy16-dis.c \
	xstormy16-ibld.c \
	xstormy16-opc.c \
	xtensa-dis.c \
	xgate-dis.c \
	xgate-opc.c \
	z80-dis.c \
	z8k-dis.c

# C source files that correspond to .o's ending up in libopcodes.
LIBOPCODES_CFILES = \
	$(TARGET32_LIBOPCODES_CFILES) \
	$(TARGET64_LIBOPCODES_CFILES) \
	dis-buf.c \
	dis-init.c \
	disassemble.c

ALL32_MACHINES = $(TARGET32_LIBOPCODES_CFILES:.c=.lo)
ALL64_MACHINES = $(TARGET64_LIBOPCODES_CFILES:.c=.lo)

OFILES = @BFD_MACHINES@

# development.sh is used to determine -Werror default.
CONFIG_STATUS_DEPENDENCIES = $(BFDDIR)/development.sh

AM_CPPFLAGS = -I. -I$(srcdir) -I../bfd -I$(INCDIR) -I$(BFDDIR) @HDEFINES@ @INCINTL@

disassemble.lo: disassemble.c
if am__fastdepCC
	$(AM_V_CC)$(LTCOMPILE) -MT $@ -MD -MP -MF $(DEPDIR)/$*.Tpo -c -o $@ @archdefs@ $(srcdir)/disassemble.c
	$(AM_V_at)mv -f $(DEPDIR)/$*.Tpo $(DEPDIR)/$*.Plo
else
if AMDEP
	source='disassemble.c' object='$@' libtool=yes @AMDEPBACKSLASH@
	DEPDIR=$(DEPDIR) $(CCDEPMODE) $(depcomp) @AMDEPBACKSLASH@
endif
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ @archdefs@ $(srcdir)/disassemble.c
endif

libopcodes_la_SOURCES =  dis-buf.c disassemble.c dis-init.c
libopcodes_la_DEPENDENCIES = $(OFILES) @SHARED_DEPENDENCIES@
libopcodes_la_LIBADD = $(OFILES) @SHARED_LIBADD@
libopcodes_la_LDFLAGS += -release `cat ../bfd/libtool-soversion` @SHARED_LDFLAGS@
# Allow dependency tracking to work on all the source files.
EXTRA_libopcodes_la_SOURCES = $(LIBOPCODES_CFILES)

# libtool will build .libs/libopcodes.a.  We create libopcodes.a in
# the build directory so that we don't have to convert all the
# programs that use libopcodes.a simultaneously.  This is a hack which
# should be removed if everything else starts using libtool.  FIXME.

noinst_LIBRARIES = libopcodes.a
libopcodes_a_SOURCES =

stamp-lib: libopcodes.la
	libtooldir=`$(LIBTOOL) --config | sed -n -e 's/^objdir=//p'`; \
	if [ -f $$libtooldir/libopcodes.a ]; then \
	  cp $$libtooldir/libopcodes.a libopcodes.tmp; \
	  $(RANLIB) libopcodes.tmp; \
	  $(SHELL) $(srcdir)/../move-if-change libopcodes.tmp libopcodes.a; \
	else true; fi
	touch stamp-lib

libopcodes.a: stamp-lib ; @true

POTFILES = $(HFILES) $(LIBOPCODES_CFILES)
po/POTFILES.in: @MAINT@ Makefile
	for f in $(POTFILES); do echo $$f; done | LC_ALL=C sort > tmp \
	  && mv tmp $(srcdir)/po/POTFILES.in

CLEANFILES = \
	stamp-epiphany stamp-fr30 stamp-frv stamp-ip2k stamp-iq2000 stamp-lm32 \
	stamp-m32c stamp-m32r stamp-mep stamp-mt stamp-or1k stamp-xstormy16 \
	libopcodes.a stamp-lib


CGENDIR = @cgendir@
CPUDIR = $(srcdir)/../cpu
GUILE = `if test -f ../guile/libguile/guile ; then echo ../guile/libguile/guile; else echo guile ; fi`
CGEN = "$(GUILE) -l ${cgendir}/guile.scm -s"
CGENFLAGS = -v

CGENDEPS = \
	$(CGENDIR)/desc.scm $(CGENDIR)/desc-cpu.scm \
	$(CGENDIR)/opcodes.scm $(CGENDIR)/opc-asmdis.scm \
	$(CGENDIR)/opc-ibld.scm $(CGENDIR)/opc-itab.scm \
	$(CGENDIR)/opc-opinst.scm \
	cgen-asm.in cgen-dis.in cgen-ibld.in

CGEN_CPUS = cris epiphany fr30 frv ip2k iq2000 lm32 m32c m32r mep mt or1k xstormy16

if CGEN_MAINT
BPF_DEPS = stamp-bpf
CRIS_DEPS = stamp-cris
EPIPHANY_DEPS = stamp-epiphany
FR30_DEPS = stamp-fr30
FRV_DEPS = stamp-frv
IP2K_DEPS = stamp-ip2k
IQ2000_DEPS = stamp-iq2000
LM32_DEPS = stamp-lm32
M32C_DEPS = stamp-m32c
M32R_DEPS = stamp-m32r
MEP_DEPS = stamp-mep
MT_DEPS = stamp-mt
OR1K_DEPS = stamp-or1k
XSTORMY16_DEPS = stamp-xstormy16
else
BPF_DEPS =
CRIS_DEPS =
EPIPHANY_DEPS =
FR30_DEPS =
FRV_DEPS =
IP2K_DEPS =
IQ2000_DEPS =
LM32_DEPS =
M32C_DEPS =
M32R_DEPS =
MEP_DEPS =
MT_DEPS =
OR1K_DEPS =
XSTORMY16_DEPS =
endif

run-cgen:
	$(SHELL) $(srcdir)/cgen.sh opcodes $(srcdir) $(CGEN) \
	  $(CGENDIR) "$(CGENFLAGS)" $(arch) $(prefix) $(archfile) $(opcfile) \
	  "$(options)" "$(extrafiles)"
	touch stamp-${prefix}
.PHONY: run-cgen

# Maintainer utility rule to regenerate all cgen files.
run-cgen-all:
	for c in $(CGEN_CPUS) ; \
	do \
		$(MAKE) stamp-$$c || exit 1 ; \
	done
.PHONY: run-cgen-all

# For now, require developers to configure with --enable-cgen-maint.

$(srcdir)/bpf-desc.h $(srcdir)/bpf-desc.c $(srcdir)/bpf-opc.h \
		$(srcdir)/bpf-opc.c $(srcdir)/bpf-ibld.c \
		$(srcdir)/bpf-asm.c $(srcdir)/bpf-dis.c: $(BPF_DEPS)
	@true

stamp-bpf: $(CGENDEPS) $(CPUDIR)/bpf.cpu $(CPUDIR)/bpf.opc
	$(MAKE) run-cgen arch=bpf prefix=bpf \
		archfile=$(CPUDIR)/bpf.cpu opcfile=$(CPUDIR)/bpf.opc

$(srcdir)/cris-desc.h $(srcdir)/cris-desc.c $(srcdir)/cris-opc.h: $(CRIS_DEPS)
	@true

stamp-cris: $(CGENDEPS) $(CPUDIR)/cris.cpu
	$(SHELL) $(srcdir)/cgen.sh desc $(srcdir) $(CGEN) \
	  $(CGENDIR) "$(CGENFLAGS)" cris cris $(CPUDIR)/cris.cpu /dev/null \
	  "$(options)" "$(extrafiles)"
	touch $@

$(srcdir)/epiphany-desc.h $(srcdir)/epiphany-desc.c $(srcdir)/epiphany-opc.h \
		$(srcdir)/epiphany-opc.c $(srcdir)/epiphany-ibld.c \
		$(srcdir)/epiphany-opinst.c $(srcdir)/epiphany-asm.c \
		$(srcdir)/epiphany-dis.c: $(EPIPHANY_DEPS)
	@true

stamp-epiphany: $(CGENDEPS) $(CPUDIR)/epiphany.cpu $(CPUDIR)/epiphany.opc
	$(MAKE) run-cgen arch=epiphany prefix=epiphany options= \
		archfile=$(CPUDIR)/epiphany.cpu opcfile=$(CPUDIR)/epiphany.opc extrafiles=

$(srcdir)/fr30-desc.h $(srcdir)/fr30-desc.c $(srcdir)/fr30-opc.h $(srcdir)/fr30-opc.c $(srcdir)/fr30-ibld.c $(srcdir)/fr30-asm.c $(srcdir)/fr30-dis.c: $(FR30_DEPS)
	@true
stamp-fr30: $(CGENDEPS) $(CPUDIR)/fr30.cpu $(CPUDIR)/fr30.opc
	$(MAKE) run-cgen arch=fr30 prefix=fr30 options= \
		archfile=$(CPUDIR)/fr30.cpu opcfile=$(CPUDIR)/fr30.opc extrafiles=

$(srcdir)/frv-desc.h $(srcdir)/frv-desc.c $(srcdir)/frv-opc.h $(srcdir)/frv-opc.c $(srcdir)/frv-ibld.c $(srcdir)/frv-asm.c $(srcdir)/frv-dis.c: $(FRV_DEPS)
	@true
stamp-frv: $(CGENDEPS) $(CPUDIR)/frv.cpu $(CPUDIR)/frv.opc
	$(MAKE) run-cgen arch=frv prefix=frv options= \
		archfile=$(CPUDIR)/frv.cpu opcfile=$(CPUDIR)/frv.opc extrafiles=

$(srcdir)/ip2k-desc.h $(srcdir)/ip2k-desc.c $(srcdir)/ip2k-opc.h $(srcdir)/ip2k-opc.c $(srcdir)/ip2k-ibld.c $(srcdir)/ip2k-asm.c $(srcdir)/ip2k-dis.c: $(IP2K_DEPS)
	@true
stamp-ip2k: $(CGENDEPS) $(CPUDIR)/ip2k.cpu $(CPUDIR)/ip2k.opc
	$(MAKE) run-cgen arch=ip2k prefix=ip2k options= \
		archfile=$(CPUDIR)/ip2k.cpu opcfile=$(CPUDIR)/ip2k.opc extrafiles=

$(srcdir)/iq2000-desc.h $(srcdir)/iq2000-desc.c $(srcdir)/iq2000-opc.h $(srcdir)/iq2000-opc.c $(srcdir)/iq2000-ibld.c $(srcdir)/iq2000-asm.c $(srcdir)/iq2000-dis.c: $(IQ2000_DEPS)
	@true
stamp-iq2000: $(CGENDEPS) $(CPUDIR)/iq2000.cpu \
		$(CPUDIR)/iq2000.opc $(CPUDIR)/iq2000m.cpu \
		$(CPUDIR)/iq10.cpu
	$(MAKE) run-cgen arch=iq2000 prefix=iq2000 options= \
		archfile=$(CPUDIR)/iq2000.cpu \
		opcfile=$(CPUDIR)/iq2000.opc extrafiles=

$(srcdir)lm32-desc.h $(srcdir)/lm32-desc.c $(srcdir)/lm32-opc.h $(srcdir)/lm32-opc.c $(srcdir)/lm32-ibld.c $(srcdir)/lm32-opinst.c $(srcdir)/lm32-asm.c $(srcdir)/lm32-dis.c: $(LM32_DEPS)
	@true
stamp-lm32: $(CGENDEPS) $(CPUDIR)/lm32.cpu $(CPUDIR)/lm32.opc
	$(MAKE) run-cgen arch=lm32 prefix=lm32 options=opinst \
		archfile=$(CPUDIR)/lm32.cpu \
		opcfile=$(CPUDIR)/lm32.opc \
		extrafiles=opinst

$(srcdir)/m32c-desc.h $(srcdir)/m32c-desc.c $(srcdir)/m32c-opc.h $(srcdir)/m32c-opc.c $(srcdir)/m32c-ibld.c $(srcdir)/m32c-asm.c $(srcdir)/m32c-dis.c: $(M32C_DEPS)
	@true
stamp-m32c: $(CGENDEPS) $(CPUDIR)/m32c.cpu $(CPUDIR)/m32c.opc
	$(MAKE) run-cgen arch=m32c prefix=m32c options= \
		archfile=$(CPUDIR)/m32c.cpu \
		opcfile=$(CPUDIR)/m32c.opc extrafiles=

$(srcdir)/m32r-desc.h $(srcdir)/m32r-desc.c $(srcdir)/m32r-opc.h $(srcdir)/m32r-opc.c $(srcdir)/m32r-ibld.c $(srcdir)/m32r-opinst.c $(srcdir)/m32r-asm.c $(srcdir)/m32r-dis.c: $(M32R_DEPS)
	@true
stamp-m32r: $(CGENDEPS) $(CPUDIR)/m32r.cpu $(CPUDIR)/m32r.opc
	$(MAKE) run-cgen arch=m32r prefix=m32r options=opinst \
		archfile=$(CPUDIR)/m32r.cpu \
		opcfile=$(CPUDIR)/m32r.opc extrafiles=opinst

$(srcdir)/mep-desc.h $(srcdir)/mep-desc.c $(srcdir)/mep-opc.h $(srcdir)/mep-opc.c $(srcdir)/mep-ibld.c $(srcdir)/mep-asm.c $(srcdir)/mep-dis.c: $(MEP_DEPS)
	@true
stamp-mep: $(CGENDEPS) $(CPUDIR)/mep.cpu $(CPUDIR)/mep-default.cpu $(CPUDIR)/mep-core.cpu $(CPUDIR)/mep-h1.cpu $(CPUDIR)/mep-ext-cop.cpu $(CPUDIR)/mep-sample-ucidsp.cpu $(CPUDIR)/mep-rhcop.cpu $(CPUDIR)/mep-fmax.cpu $(CPUDIR)/mep.opc
	$(MAKE) run-cgen arch=mep prefix=mep options= \
		archfile=$(CPUDIR)/mep.cpu opcfile=$(CPUDIR)/mep.opc extrafiles=

$(srcdir)/mt-desc.h $(srcdir)/mt-desc.c $(srcdir)/mt-opc.h $(srcdir)/mt-opc.c $(srcdir)/mt-ibld.c $(srcdir)/mt-asm.c $(srcdir)/mt-dis.c: $(MT_DEPS)
	@true
stamp-mt: $(CGENDEPS) $(CPUDIR)/mt.cpu $(CPUDIR)/mt.opc
	$(MAKE) run-cgen arch=mt prefix=mt options= \
		archfile=$(CPUDIR)/mt.cpu \
		opcfile=$(CPUDIR)/mt.opc extrafiles=

$(srcdir)/or1k-desc.h $(srcdir)/or1k-desc.c $(srcdir)/or1k-opc.h $(srcdir)/or1k-opc.c $(srcdir)/or1k-ibld.c $(srcdir)/or1k-opinst.c $(srcdir)/or1k-asm.c $(srcdir)/or1k-dis.c: $(OR1K_DEPS)
	@true
stamp-or1k: $(CGENDEPS) $(CPUDIR)/or1k.cpu $(CPUDIR)/or1k.opc $(CPUDIR)/or1kcommon.cpu $(CPUDIR)/or1korbis.cpu $(CPUDIR)/or1korfpx.cpu
	$(MAKE) run-cgen arch=or1k prefix=or1k options=opinst \
		archfile=$(CPUDIR)/or1k.cpu opcfile=$(CPUDIR)/or1k.opc extrafiles=opinst

$(srcdir)/xstormy16-desc.h $(srcdir)/xstormy16-desc.c $(srcdir)/xstormy16-opc.h $(srcdir)/xstormy16-opc.c $(srcdir)/xstormy16-ibld.c $(srcdir)/xstormy16-asm.c $(srcdir)/xstormy16-dis.c: $(XSTORMY16_DEPS)
	@true
stamp-xstormy16: $(CGENDEPS) $(CPUDIR)/xstormy16.cpu $(CPUDIR)/xstormy16.opc
	$(MAKE) run-cgen arch=xstormy16 prefix=xstormy16 options= \
		archfile=$(CPUDIR)/xstormy16.cpu opcfile=$(CPUDIR)/xstormy16.opc extrafiles=

MOSTLYCLEANFILES = aarch64-gen$(EXEEXT_FOR_BUILD) i386-gen$(EXEEXT_FOR_BUILD) \
	ia64-gen$(EXEEXT_FOR_BUILD) s390-mkopc$(EXEEXT_FOR_BUILD) s390-opc.tab \
	z8kgen$(EXEEXT_FOR_BUILD) opc2c$(EXEEXT_FOR_BUILD)

MAINTAINERCLEANFILES = $(srcdir)/aarch64-asm-2.c $(srcdir)/aarch64-dis-2.c \
	$(srcdir)/aarch64-opc-2.c \
	$(srcdir)/i386-tbl.h $(srcdir)/i386-init.h $(srcdir)/i386-mnem.h \
	$(srcdir)/ia64-asmtab.c $(srcdir)/z8k-opc.h \
	$(srcdir)/msp430-decode.c \
	$(srcdir)/rl78-decode.c \
	$(srcdir)/rx-decode.c

aarch64-gen$(EXEEXT_FOR_BUILD): aarch64-gen.o $(BUILD_LIB_DEPS)
	$(LINK_FOR_BUILD) aarch64-gen.o $(BUILD_LIBS)

aarch64-gen.o: aarch64-gen.c $(BFD_H) $(INCDIR)/getopt.h $(INCDIR)/libiberty.h\
	$(INCDIR)/opcode/aarch64.h config.h aarch64-opc.h aarch64-tbl.h
	$(AM_V_CC)$(COMPILE_FOR_BUILD) -c $(srcdir)/aarch64-gen.c

$(srcdir)/aarch64-asm-2.c: @MAINT@ aarch64-gen$(exeext_for_build)
	$(AM_V_GEN)./aarch64-gen$(exeext_for_build) --gen-asm > $@
$(srcdir)/aarch64-dis-2.c: @MAINT@ aarch64-gen$(exeext_for_build)
	$(AM_V_GEN)./aarch64-gen$(exeext_for_build) --gen-dis > $@
$(srcdir)/aarch64-opc-2.c: @MAINT@ aarch64-gen$(exeext_for_build)
	$(AM_V_GEN)./aarch64-gen$(exeext_for_build) --gen-opc > $@

i386-gen$(EXEEXT_FOR_BUILD): i386-gen.o $(BUILD_LIB_DEPS)
	$(AM_V_CCLD)$(LINK_FOR_BUILD) i386-gen.o $(BUILD_LIBS)

i386-gen.o: i386-gen.c i386-opc.h $(srcdir)/../include/opcode/i386.h \
	$(INCDIR)/ansidecl.h $(INCDIR)/getopt.h $(INCDIR)/hashtab.h \
	$(INCDIR)/libiberty.h $(INCDIR)/opcode/i386.h $(INCDIR)/safe-ctype.h \
	config.h sysdep.h
	$(COMPILE_FOR_BUILD) -c $(srcdir)/i386-gen.c

# i386-gen will generate all headers in one go.  Use a pattern rule to properly
# express this, with the inner dash ('-') arbitrarily chosen to be the stem.
$(srcdir)/i386%tbl.h $(srcdir)/i386%init.h $(srcdir)/i386%mnem.h: \
		@MAINT@ i386-gen$(EXEEXT_FOR_BUILD) i386-opc.tbl i386-reg.tbl i386-opc.h
	$(AM_V_GEN)$(CPP) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) - \
		< $(srcdir)/i386-opc.tbl \
		| ./i386-gen$(EXEEXT_FOR_BUILD) --srcdir $(srcdir)

# While not really dependencies, specify other generated i386-*.h here as well
# to make sure they are re-generated as necessary.
i386-dis.lo: $(srcdir)/i386-tbl.h $(srcdir)/i386-init.h $(srcdir)/i386-mnem.h

ia64-gen$(EXEEXT_FOR_BUILD): ia64-gen.o $(BUILD_LIB_DEPS)
	$(AM_V_CCLD)$(LINK_FOR_BUILD) ia64-gen.o $(BUILD_LIBS)

ia64-gen.o: ia64-gen.c $(BFD_H) $(INCDIR)/ansidecl.h $(INCDIR)/getopt.h \
	$(INCDIR)/libiberty.h $(INCDIR)/opcode/ia64.h $(INCDIR)/safe-ctype.h \
	$(INCDIR)/symcat.h config.h ia64-opc-a.c ia64-opc-b.c \
	ia64-opc-d.c ia64-opc-f.c ia64-opc-i.c ia64-opc-m.c \
	ia64-opc-x.c ia64-opc.h sysdep.h
	$(COMPILE_FOR_BUILD) -c $(srcdir)/ia64-gen.c

# Use a helper variable for the dependencies to avoid 'make' issues
# with continuations in comments, as @MAINT@ can be expanded to '#'.
ia64_asmtab_deps = ia64-gen$(EXEEXT_FOR_BUILD) ia64-ic.tbl \
	ia64-raw.tbl ia64-waw.tbl ia64-war.tbl
$(srcdir)/ia64-asmtab.c: @MAINT@ $(ia64_asmtab_deps)
	$(AM_V_GEN)./ia64-gen$(EXEEXT_FOR_BUILD) --srcdir $(srcdir) > $@

ia64-opc.lo: $(srcdir)/ia64-asmtab.c

$(srcdir)/msp430-decode.c: @MAINT@ $(srcdir)/msp430-decode.opc opc2c$(EXEEXT_FOR_BUILD)
	$(AM_V_GEN)./opc2c$(EXEEXT_FOR_BUILD) $(srcdir)/msp430-decode.opc > $(srcdir)/msp430-decode.c

$(srcdir)/rl78-decode.c: @MAINT@ $(srcdir)/rl78-decode.opc opc2c$(EXEEXT_FOR_BUILD)
	$(AM_V_GEN)./opc2c$(EXEEXT_FOR_BUILD) $(srcdir)/rl78-decode.opc > $(srcdir)/rl78-decode.c

$(srcdir)/rx-decode.c: @MAINT@ $(srcdir)/rx-decode.opc opc2c$(EXEEXT_FOR_BUILD)
	$(AM_V_GEN)./opc2c$(EXEEXT_FOR_BUILD) $(srcdir)/rx-decode.opc > $(srcdir)/rx-decode.c

opc2c$(EXEEXT_FOR_BUILD): opc2c.o $(BUILD_LIBS_DEPS)
	$(AM_V_CCLD)$(LINK_FOR_BUILD) opc2c.o $(BUILD_LIBS)

opc2c.o: opc2c.c $(INCDIR)/libiberty.h
	$(AM_V_CC)$(COMPILE_FOR_BUILD) -c $(srcdir)/opc2c.c

s390-mkopc$(EXEEXT_FOR_BUILD): s390-mkopc.c
	$(AM_V_CCLD)$(COMPILE_FOR_BUILD) -o s390-mkopc $(srcdir)/s390-mkopc.c

s390-opc.tab: s390-mkopc$(EXEEXT_FOR_BUILD) s390-opc.txt
	$(AM_V_GEN)./s390-mkopc$(EXEEXT_FOR_BUILD) < $(srcdir)/s390-opc.txt > s390-opc.tab

s390-opc.lo: s390-opc.tab

z8kgen$(EXEEXT_FOR_BUILD): z8kgen.o $(BUILD_LIB_DEPS)
	$(AM_V_CCLD)$(LINK_FOR_BUILD) z8kgen.o $(BUILD_LIBS)

z8kgen.o: z8kgen.c
	$(AM_V_CC)$(COMPILE_FOR_BUILD) -c $(srcdir)/z8kgen.c

$(srcdir)/z8k-opc.h: @MAINT@ z8kgen$(EXEEXT_FOR_BUILD)
	$(AM_V_GEN)./z8kgen$(EXEEXT_FOR_BUILD) -a > $@

z8k-dis.lo: $(srcdir)/z8k-opc.h

MIPS_DEFS=`case \`cat ../bfd/ofiles\` in *elfxx-mips*) echo "-DHAVE_BFD_MIPS_ELF_GET_ABIFLAGS=1";; esac`
mips-dis.lo: mips-dis.c
if am__fastdepCC
	$(AM_V_CC)$(LTCOMPILE) -MT $@ -MD -MP -MF $(DEPDIR)/$*.Tpo -c -o $@ $(MIPS_DEFS) $<
	$(AM_V_at)$(am__mv) $(DEPDIR)/$*.Tpo $(DEPDIR)/$*.Plo
else
if AMDEP
	source='$<' object='$@' libtool=yes @AMDEPBACKSLASH@
	DEPDIR=$(DEPDIR) $(CCDEPMODE) $(depcomp) @AMDEPBACKSLASH@
endif
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $(MIPS_DEFS) $<
endif

sh-dis.lo: sh-dis.c
if am__fastdepCC
	$(AM_V_CC)$(LTCOMPILE) -MT $@ -MD -MP -MF $(DEPDIR)/$*.Tpo -c -o $@ @archdefs@ $(srcdir)/sh-dis.c
	$(AM_V_at)mv -f $(DEPDIR)/$*.Tpo $(DEPDIR)/$*.Plo
else
if AMDEP
	source='sh-dis.c' object='$@' libtool=yes @AMDEPBACKSLASH@
	DEPDIR=$(DEPDIR) $(CCDEPMODE) $(depcomp) @AMDEPBACKSLASH@
endif
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ @archdefs@ $(srcdir)/sh-dis.c
endif
