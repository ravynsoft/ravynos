# -*- makefile -*-
#
# Makefile for building NASM using OpenWatcom
# cross-compile on a DOS/Win32/OS2 platform host
#

top_srcdir  = .
srcdir      = .
VPATH       = $(srcdir)\asm;$(srcdir)\x86;asm;x86;$(srcdir)\macros;macros;$(srcdir)\output;$(srcdir)\lib;$(srcdir)\common;$(srcdir)\stdlib;$(srcdir)\nasmlib;$(srcdir)\disasm
prefix      = C:\Program Files\NASM
exec_prefix = $(prefix)
bindir      = $(prefix)\bin
mandir      = $(prefix)\man

CC      = *wcl386
DEBUG       =
CFLAGS      = -zq -6 -ox -wx -wcd=124 -ze -fpi $(DEBUG)
BUILD_CFLAGS    = $(CFLAGS) $(%TARGET_CFLAGS)
INTERNAL_CFLAGS = -I$(srcdir) -I. -I$(srcdir)\include -I$(srcdir)\x86 -Ix86 -I$(srcdir)\asm -Iasm -I$(srcdir)\disasm -I$(srcdir)\output
ALL_CFLAGS  = $(BUILD_CFLAGS) $(INTERNAL_CFLAGS)
LD      = *wlink
LDEBUG      =
LDFLAGS     = op q $(%TARGET_LFLAGS) $(LDEBUG)
LIBS        =
STRIP       = wstrip

PERL		= perl
PERLFLAGS	= -I$(srcdir)\perllib -I$(srcdir)
RUNPERL         = $(PERL) $(PERLFLAGS)

MAKENSIS        = makensis

# Binary suffixes
O               = obj
X               = .exe

# WMAKE errors out if a suffix is declared more than once, including
# its own built-in declarations.  Thus, we need to explicitly clear the list
# first.  Also, WMAKE only allows implicit rules that point "to the left"
# in this list!
.SUFFIXES:
.SUFFIXES: .man .1 .$(O) .i .c

# Needed to find C files anywhere but in the current directory
.c : $(VPATH)

.c.$(O):
    @set INCLUDE=
    $(CC) -c $(ALL_CFLAGS) -fo=$^@ $[@

#-- Begin File Lists --#
# Edit in Makefile.in, not here!
NASM    = asm\nasm.$(O)
NDISASM = disasm\ndisasm.$(O)

PROGOBJ = $(NASM) $(NDISASM)
PROGS   = nasm$(X) ndisasm$(X)

LIBOBJ_NW = stdlib\snprintf.$(O) stdlib\vsnprintf.$(O) stdlib\strlcpy.$(O) &
	stdlib\strnlen.$(O) stdlib\strrchrnul.$(O) &
	&
	nasmlib\ver.$(O) &
	nasmlib\alloc.$(O) nasmlib\asprintf.$(O) nasmlib\errfile.$(O) &
	nasmlib\crc32.$(O) nasmlib\crc64.$(O) nasmlib\md5c.$(O) &
	nasmlib\string.$(O) nasmlib\nctype.$(O) &
	nasmlib\file.$(O) nasmlib\mmap.$(O) nasmlib\ilog2.$(O) &
	nasmlib\realpath.$(O) nasmlib\path.$(O) &
	nasmlib\filename.$(O) nasmlib\rlimit.$(O) &
	nasmlib\zerobuf.$(O) nasmlib\readnum.$(O) nasmlib\bsi.$(O) &
	nasmlib\rbtree.$(O) nasmlib\hashtbl.$(O) &
	nasmlib\raa.$(O) nasmlib\saa.$(O) &
	nasmlib\strlist.$(O) &
	nasmlib\perfhash.$(O) nasmlib\badenum.$(O) &
	&
	common\common.$(O) &
	&
	x86\insnsa.$(O) x86\insnsb.$(O) x86\insnsd.$(O) x86\insnsn.$(O) &
	x86\regs.$(O) x86\regvals.$(O) x86\regflags.$(O) x86\regdis.$(O) &
	x86\disp8.$(O) x86\iflag.$(O) &
	&
	asm\error.$(O) &
	asm\floats.$(O) &
	asm\directiv.$(O) asm\directbl.$(O) &
	asm\pragma.$(O) &
	asm\assemble.$(O) asm\labels.$(O) asm\parser.$(O) &
	asm\preproc.$(O) asm\quote.$(O) asm\pptok.$(O) &
	asm\listing.$(O) asm\eval.$(O) asm\exprlib.$(O) asm\exprdump.$(O) &
	asm\stdscan.$(O) &
	asm\strfunc.$(O) asm\tokhash.$(O) &
	asm\segalloc.$(O) &
	asm\rdstrnum.$(O) &
	asm\srcfile.$(O) &
	macros\macros.$(O) &
	&
	output\outform.$(O) output\outlib.$(O) output\legacy.$(O) &
	output\nulldbg.$(O) output\nullout.$(O) &
	output\outbin.$(O) output\outaout.$(O) output\outcoff.$(O) &
	output\outelf.$(O) &
	output\outobj.$(O) output\outas86.$(O) &
	output\outdbg.$(O) output\outieee.$(O) output\outmacho.$(O) &
	output\codeview.$(O) &
	&
	disasm\disasm.$(O) disasm\sync.$(O)

# Warnings depend on all source files, so handle them separately
WARNOBJ   = asm\warnings.$(O)

LIBOBJ    = $(LIBOBJ_NW) $(WARNOBJ)
ALLOBJ_NW = $(PROGOBJ) $(LIBOBJ_NW)
ALLOBJ    = $(PROGOBJ) $(LIBOBJ)

SUBDIRS  = stdlib nasmlib output asm disasm x86 common macros
XSUBDIRS = test doc nsis
DEPDIRS  = . include config x86 $(SUBDIRS)
#-- End File Lists --#

what:   .SYMBOLIC
    @echo Please build "dos", "win32", "os2" or "linux386"

dos:    .SYMBOLIC
    @set TARGET_CFLAGS=-bt=DOS -I"$(%WATCOM)\h"
    @set TARGET_LFLAGS=sys causeway
    @%make all

win32:  .SYMBOLIC
    @set TARGET_CFLAGS=-bt=NT -I"$(%WATCOM)\h" -I"$(%WATCOM)\h\nt"
    @set TARGET_LFLAGS=sys nt
    @%make all

os2:    .SYMBOLIC
    @set TARGET_CFLAGS=-bt=OS2 -I"$(%WATCOM)\h" -I"$(%WATCOM)\h\os2"
    @set TARGET_LFLAGS=sys os2v2
    @%make all

linux386:   .SYMBOLIC
    @set TARGET_CFLAGS=-bt=LINUX -I"$(%WATCOM)\lh"
    @set TARGET_LFLAGS=sys linux
    @%make all

all: perlreq nasm$(X) ndisasm$(X) .SYMBOLIC
#   cd rdoff && $(MAKE) all

NASMLIB = nasm.lib

nasm$(X): $(NASM) $(NASMLIB)
    $(LD) $(LDFLAGS) name nasm$(X) libr {$(NASMLIB) $(LIBS)} file {$(NASM)}

ndisasm$(X): $(NDISASM) $(LIBOBJ)
    $(LD) $(LDFLAGS) name ndisasm$(X) libr {$(NASMLIB) $(LIBS)} file {$(NDISASM)}

nasm.lib: $(LIBOBJ)
    wlib -q -b -n $@ $(LIBOBJ)

#-- Begin Generated File Rules --#
# Edit in Makefile.in, not here!

# These source files are automagically generated from data files using
# Perl scripts. They're distributed, though, so it isn't necessary to
# have Perl just to recompile NASM from the distribution.

# Perl-generated source files
PERLREQ = config\unconfig.h &
	  x86\insnsb.c x86\insnsa.c x86\insnsd.c x86\insnsi.h x86\insnsn.c &
	  x86\regs.c x86\regs.h x86\regflags.c x86\regdis.c x86\regdis.h &
	  x86\regvals.c asm\tokhash.c asm\tokens.h asm\pptok.h asm\pptok.c &
	  x86\iflag.c x86\iflaggen.h &
	  macros\macros.c &
	  asm\pptok.ph asm\directbl.c asm\directiv.h &
	  asm\warnings.c include\warnings.h doc\warnings.src &
	  misc\nasmtok.el &
	  version.h version.mac version.mak nsis\version.nsh

INSDEP = x86\insns.dat x86\insns.pl x86\insns-iflags.ph x86\iflags.ph

config\unconfig.h: config\config.h.in
	$(RUNPERL) $(srcdir)\tools\unconfig.pl &
		'$(srcdir)' config\config.h.in config\unconfig.h

x86\iflag.c: $(INSDEP)
	$(RUNPERL) $(srcdir)\x86\insns.pl -fc &
		$(srcdir)\x86\insns.dat x86\iflag.c
x86\iflaggen.h: $(INSDEP)
	$(RUNPERL) $(srcdir)\x86\insns.pl -fh &
		$(srcdir)\x86\insns.dat x86\iflaggen.h
x86\insnsb.c: $(INSDEP)
	$(RUNPERL) $(srcdir)\x86\insns.pl -b &
		$(srcdir)\x86\insns.dat x86\insnsb.c
x86\insnsa.c: $(INSDEP)
	$(RUNPERL) $(srcdir)\x86\insns.pl -a &
		$(srcdir)\x86\insns.dat x86\insnsa.c
x86\insnsd.c: $(INSDEP)
	$(RUNPERL) $(srcdir)\x86\insns.pl -d &
		$(srcdir)\x86\insns.dat x86\insnsd.c
x86\insnsi.h: $(INSDEP)
	$(RUNPERL) $(srcdir)\x86\insns.pl -i &
		$(srcdir)\x86\insns.dat x86\insnsi.h
x86\insnsn.c: $(INSDEP)
	$(RUNPERL) $(srcdir)\x86\insns.pl -n &
		$(srcdir)\x86\insns.dat x86\insnsn.c

# These files contains all the standard macros that are derived from
# the version number.
version.h: version version.pl
	$(RUNPERL) $(srcdir)\version.pl h < $(srcdir)\version > version.h
version.mac: version version.pl
	$(RUNPERL) $(srcdir)\version.pl mac < $(srcdir)\version > version.mac
version.sed: version version.pl
	$(RUNPERL) $(srcdir)\version.pl sed < $(srcdir)\version > version.sed
version.mak: version version.pl
	$(RUNPERL) $(srcdir)\version.pl make < $(srcdir)\version > version.mak
nsis\version.nsh: version version.pl
	$(RUNPERL) $(srcdir)\version.pl nsis < $(srcdir)\version > nsis\version.nsh

# This source file is generated from the standard macros file
# `standard.mac' by another Perl script. Again, it's part of the
# standard distribution.
macros\macros.c: macros\macros.pl asm\pptok.ph version.mac &
	$(srcdir)\macros\*.mac $(srcdir)\output\*.mac
	$(RUNPERL) $(srcdir)\macros\macros.pl version.mac &
		$(srcdir)\macros\*.mac $(srcdir)\output\*.mac

# These source files are generated from regs.dat by yet another
# perl script.
x86\regs.c: x86\regs.dat x86\regs.pl
	$(RUNPERL) $(srcdir)\x86\regs.pl c &
		$(srcdir)\x86\regs.dat > x86\regs.c
x86\regflags.c: x86\regs.dat x86\regs.pl
	$(RUNPERL) $(srcdir)\x86\regs.pl fc &
		$(srcdir)\x86\regs.dat > x86\regflags.c
x86\regdis.c: x86\regs.dat x86\regs.pl
	$(RUNPERL) $(srcdir)\x86\regs.pl dc &
		$(srcdir)\x86\regs.dat > x86\regdis.c
x86\regdis.h: x86\regs.dat x86\regs.pl
	$(RUNPERL) $(srcdir)\x86\regs.pl dh &
		$(srcdir)\x86\regs.dat > x86\regdis.h
x86\regvals.c: x86\regs.dat x86\regs.pl
	$(RUNPERL) $(srcdir)\x86\regs.pl vc &
		$(srcdir)\x86\regs.dat > x86\regvals.c
x86\regs.h: x86\regs.dat x86\regs.pl
	$(RUNPERL) $(srcdir)\x86\regs.pl h &
		$(srcdir)\x86\regs.dat > x86\regs.h

# Extract warnings from source code. This is done automatically if any
# C files have changed; the script is fast enough that that is
# reasonable, but doesn't update the time stamp if the files aren't
# changed, to avoid rebuilding everything every time. Track the actual
# dependency by the empty file asm\warnings.time.
WARNFILES = asm\warnings.c include\warnings.h doc\warnings.src

warnings:
	$(RM_F) $(WARNFILES) $(WARNFILES:=.time)
	$(MAKE) asm\warnings.time

asm\warnings.time: $(ALLOBJ_NW:.$(O)=.c)
	: > asm\warnings.time
	$(MAKE) $(WARNFILES:=.time)

asm\warnings.c.time: asm\warnings.pl asm\warnings.time
	$(RUNPERL) $(srcdir)\asm\warnings.pl c asm\warnings.c $(srcdir)
	: > asm\warnings.c.time

asm\warnings.c: asm\warnings.c.time
	@: Side effect

include\warnings.h.time: asm\warnings.pl asm\warnings.time
	$(RUNPERL) $(srcdir)\asm\warnings.pl h include\warnings.h $(srcdir)
	: > include\warnings.h.time

include\warnings.h: include\warnings.h.time
	@: Side effect

doc\warnings.src.time: asm\warnings.pl asm\warnings.time
	$(RUNPERL) $(srcdir)\asm\warnings.pl doc doc\warnings.src $(srcdir)
	: > doc\warnings.src.time

doc\warnings.src : doc\warnings.src.time
	@: Side effect

# Assembler token hash
asm\tokhash.c: x86\insns.dat x86\insnsn.c asm\tokens.dat asm\tokhash.pl &
	perllib\phash.ph
	$(RUNPERL) $(srcdir)\asm\tokhash.pl c &
		x86\insnsn.c $(srcdir)\x86\regs.dat &
		$(srcdir)\asm\tokens.dat > asm\tokhash.c

# Assembler token metadata
asm\tokens.h: x86\insns.dat x86\insnsn.c asm\tokens.dat asm\tokhash.pl &
	perllib\phash.ph
	$(RUNPERL) $(srcdir)\asm\tokhash.pl h &
		x86\insnsn.c $(srcdir)\x86\regs.dat &
		$(srcdir)\asm\tokens.dat > asm\tokens.h

# Preprocessor token hash
asm\pptok.h: asm\pptok.dat asm\pptok.pl perllib\phash.ph
	$(RUNPERL) $(srcdir)\asm\pptok.pl h &
		$(srcdir)\asm\pptok.dat asm\pptok.h
asm\pptok.c: asm\pptok.dat asm\pptok.pl perllib\phash.ph
	$(RUNPERL) $(srcdir)\asm\pptok.pl c &
		$(srcdir)\asm\pptok.dat asm\pptok.c
asm\pptok.ph: asm\pptok.dat asm\pptok.pl perllib\phash.ph
	$(RUNPERL) $(srcdir)\asm\pptok.pl ph &
		$(srcdir)\asm\pptok.dat asm\pptok.ph

# Directives hash
asm\directiv.h: asm\directiv.dat nasmlib\perfhash.pl perllib\phash.ph
	$(RUNPERL) $(srcdir)\nasmlib\perfhash.pl h &
		$(srcdir)\asm\directiv.dat asm\directiv.h
asm\directbl.c: asm\directiv.dat nasmlib\perfhash.pl perllib\phash.ph
	$(RUNPERL) $(srcdir)\nasmlib\perfhash.pl c &
		$(srcdir)\asm\directiv.dat asm\directbl.c

# Emacs token files
misc\nasmtok.el: misc\emacstbl.pl asm\tokhash.c asm\pptok.c &
		 asm\directiv.dat version
	$(RUNPERL) $< $@ "$(srcdir)" "$(objdir)"

#-- End Generated File Rules --#

perlreq: $(PERLREQ) .SYMBOLIC

#-- Begin NSIS Rules --#
# Edit in Makefile.in, not here!

nsis\arch.nsh: nsis\getpearch.pl nasm$(X)
	$(PERL) $(srcdir)\nsis\getpearch.pl nasm$(X) > nsis\arch.nsh

# Should only be done after "make everything".
# The use of redirection here keeps makensis from moving the cwd to the
# source directory.
nsis: nsis\nasm.nsi nsis\arch.nsh nsis\version.nsh
	$(MAKENSIS) -Dsrcdir="$(srcdir)" -Dobjdir="$(objdir)" - < nsis\nasm.nsi

#-- End NSIS Rules --#

clean: .SYMBOLIC
    rm -f *.$(O) *.s *.i
    rm -f asm\*.$(O) asm\*.s asm\*.i
    rm -f x86\*.$(O) x86\*.s x86\*.i
    rm -f lib\*.$(O) lib\*.s lib\*.i
    rm -f macros\*.$(O) macros\*.s macros\*.i
    rm -f output\*.$(O) output\*.s output\*.i
    rm -f common\*.$(O) common\*.s common\*.i
    rm -f stdlib\*.$(O) stdlib\*.s stdlib\*.i
    rm -f nasmlib\*.$(O) nasmlib\*.s nasmlib\*.i
    rm -f disasm\*.$(O) disasm\*.s disasm\*.i
    rm -f config.h config.log config.status
    rm -f nasm$(X) ndisasm$(X) $(NASMLIB)

distclean: clean .SYMBOLIC
    rm -f config.h config.log config.status
    rm -f Makefile *~ *.bak *.lst *.bin
    rm -f output\*~ output\*.bak
    rm -f test\*.lst test\*.bin test\*.$(O) test\*.bin

cleaner: clean .SYMBOLIC
    rm -f $(PERLREQ)
    rm -f *.man
    rm -f nasm.spec
#   cd doc && $(MAKE) clean

spotless: distclean cleaner .SYMBOLIC
    rm -f doc\Makefile doc\*~ doc\*.bak

strip: .SYMBOLIC
    $(STRIP) *.exe

doc:
#   cd doc && $(MAKE) all

everything: all doc

#
# This build dependencies in *ALL* makefiles.  Partially for that reason,
# it's expected to be invoked manually.
#
alldeps: perlreq .SYMBOLIC
    $(PERL) syncfiles.pl Makefile.in Mkfiles\openwcom.mak
    $(PERL) mkdep.pl -M Makefile.in Mkfiles\openwcom.mak -- . output lib

#-- Magic hints to mkdep.pl --#
# @object-ending: ".$(O)"
# @path-separator: "\"
# @exclude: "config/config.h"
# @continuation: "&"
#-- Everything below is generated by mkdep.pl - do not edit --#
asm\assemble.$(O): asm\assemble.c asm\assemble.h asm\directiv.h &
 asm\listing.h asm\pptok.h asm\preproc.h asm\srcfile.h asm\tokens.h &
 config\msvc.h config\unconfig.h config\unknown.h config\watcom.h &
 include\bytesex.h include\compiler.h include\dbginfo.h include\disp8.h &
 include\error.h include\hashtbl.h include\iflag.h include\ilog2.h &
 include\insns.h include\labels.h include\nasm.h include\nasmint.h &
 include\nasmlib.h include\nctype.h include\opflags.h include\perfhash.h &
 include\rbtree.h include\strlist.h include\tables.h include\warnings.h &
 x86\iflaggen.h x86\insnsi.h x86\regs.h
asm\directbl.$(O): asm\directbl.c asm\directiv.h config\msvc.h &
 config\unconfig.h config\unknown.h config\watcom.h include\bytesex.h &
 include\compiler.h include\nasmint.h include\nasmlib.h include\perfhash.h
asm\directiv.$(O): asm\directiv.c asm\assemble.h asm\directiv.h asm\eval.h &
 asm\floats.h asm\listing.h asm\pptok.h asm\preproc.h asm\srcfile.h &
 asm\stdscan.h config\msvc.h config\unconfig.h config\unknown.h &
 config\watcom.h include\bytesex.h include\compiler.h include\error.h &
 include\hashtbl.h include\iflag.h include\ilog2.h include\labels.h &
 include\nasm.h include\nasmint.h include\nasmlib.h include\nctype.h &
 include\opflags.h include\perfhash.h include\strlist.h include\tables.h &
 include\warnings.h output\outform.h x86\iflaggen.h x86\insnsi.h x86\regs.h
asm\error.$(O): asm\error.c config\msvc.h config\unconfig.h config\unknown.h &
 config\watcom.h include\bytesex.h include\compiler.h include\error.h &
 include\nasmint.h include\nasmlib.h include\warnings.h
asm\eval.$(O): asm\eval.c asm\assemble.h asm\directiv.h asm\eval.h &
 asm\floats.h asm\pptok.h asm\preproc.h asm\srcfile.h config\msvc.h &
 config\unconfig.h config\unknown.h config\watcom.h include\bytesex.h &
 include\compiler.h include\error.h include\hashtbl.h include\iflag.h &
 include\ilog2.h include\labels.h include\nasm.h include\nasmint.h &
 include\nasmlib.h include\nctype.h include\opflags.h include\perfhash.h &
 include\strlist.h include\tables.h include\warnings.h x86\iflaggen.h &
 x86\insnsi.h x86\regs.h
asm\exprdump.$(O): asm\exprdump.c asm\directiv.h asm\pptok.h asm\preproc.h &
 asm\srcfile.h config\msvc.h config\unconfig.h config\unknown.h &
 config\watcom.h include\bytesex.h include\compiler.h include\error.h &
 include\hashtbl.h include\labels.h include\nasm.h include\nasmint.h &
 include\nasmlib.h include\nctype.h include\opflags.h include\perfhash.h &
 include\strlist.h include\tables.h include\warnings.h x86\insnsi.h &
 x86\regs.h
asm\exprlib.$(O): asm\exprlib.c asm\directiv.h asm\pptok.h asm\preproc.h &
 asm\srcfile.h config\msvc.h config\unconfig.h config\unknown.h &
 config\watcom.h include\bytesex.h include\compiler.h include\error.h &
 include\hashtbl.h include\labels.h include\nasm.h include\nasmint.h &
 include\nasmlib.h include\nctype.h include\opflags.h include\perfhash.h &
 include\strlist.h include\tables.h include\warnings.h x86\insnsi.h &
 x86\regs.h
asm\floats.$(O): asm\floats.c asm\directiv.h asm\floats.h asm\pptok.h &
 asm\preproc.h asm\srcfile.h config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\error.h include\hashtbl.h include\labels.h include\nasm.h &
 include\nasmint.h include\nasmlib.h include\nctype.h include\opflags.h &
 include\perfhash.h include\strlist.h include\tables.h include\warnings.h &
 x86\insnsi.h x86\regs.h
asm\labels.$(O): asm\labels.c asm\directiv.h asm\pptok.h asm\preproc.h &
 asm\srcfile.h config\msvc.h config\unconfig.h config\unknown.h &
 config\watcom.h include\bytesex.h include\compiler.h include\error.h &
 include\hashtbl.h include\labels.h include\nasm.h include\nasmint.h &
 include\nasmlib.h include\nctype.h include\opflags.h include\perfhash.h &
 include\strlist.h include\tables.h include\warnings.h x86\insnsi.h &
 x86\regs.h
asm\listing.$(O): asm\listing.c asm\directiv.h asm\listing.h asm\pptok.h &
 asm\preproc.h asm\srcfile.h config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\error.h include\hashtbl.h include\labels.h include\nasm.h &
 include\nasmint.h include\nasmlib.h include\nctype.h include\opflags.h &
 include\perfhash.h include\strlist.h include\tables.h include\warnings.h &
 x86\insnsi.h x86\regs.h
asm\nasm.$(O): asm\nasm.c asm\assemble.h asm\directiv.h asm\eval.h &
 asm\floats.h asm\listing.h asm\parser.h asm\pptok.h asm\preproc.h &
 asm\quote.h asm\srcfile.h asm\stdscan.h asm\tokens.h config\msvc.h &
 config\unconfig.h config\unknown.h config\watcom.h include\bytesex.h &
 include\compiler.h include\error.h include\hashtbl.h include\iflag.h &
 include\ilog2.h include\insns.h include\labels.h include\nasm.h &
 include\nasmint.h include\nasmlib.h include\nctype.h include\opflags.h &
 include\perfhash.h include\raa.h include\saa.h include\strlist.h &
 include\tables.h include\ver.h include\warnings.h output\outform.h &
 x86\iflaggen.h x86\insnsi.h x86\regs.h
asm\parser.$(O): asm\parser.c asm\assemble.h asm\directiv.h asm\eval.h &
 asm\floats.h asm\parser.h asm\pptok.h asm\preproc.h asm\srcfile.h &
 asm\stdscan.h asm\tokens.h config\msvc.h config\unconfig.h config\unknown.h &
 config\watcom.h include\bytesex.h include\compiler.h include\error.h &
 include\hashtbl.h include\iflag.h include\ilog2.h include\insns.h &
 include\labels.h include\nasm.h include\nasmint.h include\nasmlib.h &
 include\nctype.h include\opflags.h include\perfhash.h include\strlist.h &
 include\tables.h include\warnings.h x86\iflaggen.h x86\insnsi.h x86\regs.h
asm\pptok.$(O): asm\pptok.c asm\pptok.h asm\preproc.h config\msvc.h &
 config\unconfig.h config\unknown.h config\watcom.h include\bytesex.h &
 include\compiler.h include\hashtbl.h include\nasmint.h include\nasmlib.h &
 include\nctype.h
asm\pragma.$(O): asm\pragma.c asm\assemble.h asm\directiv.h asm\listing.h &
 asm\pptok.h asm\preproc.h asm\srcfile.h config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\error.h include\hashtbl.h include\iflag.h include\ilog2.h &
 include\labels.h include\nasm.h include\nasmint.h include\nasmlib.h &
 include\nctype.h include\opflags.h include\perfhash.h include\strlist.h &
 include\tables.h include\warnings.h x86\iflaggen.h x86\insnsi.h x86\regs.h
asm\preproc.$(O): asm\preproc.c asm\directiv.h asm\eval.h asm\listing.h &
 asm\pptok.h asm\preproc.h asm\quote.h asm\srcfile.h asm\stdscan.h &
 asm\tokens.h config\msvc.h config\unconfig.h config\unknown.h &
 config\watcom.h include\bytesex.h include\compiler.h include\dbginfo.h &
 include\error.h include\hashtbl.h include\labels.h include\nasm.h &
 include\nasmint.h include\nasmlib.h include\nctype.h include\opflags.h &
 include\perfhash.h include\rbtree.h include\strlist.h include\tables.h &
 include\warnings.h x86\insnsi.h x86\regs.h
asm\quote.$(O): asm\quote.c asm\quote.h config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\error.h include\nasmint.h include\nasmlib.h include\nctype.h &
 include\warnings.h
asm\rdstrnum.$(O): asm\rdstrnum.c asm\directiv.h asm\pptok.h asm\preproc.h &
 asm\srcfile.h config\msvc.h config\unconfig.h config\unknown.h &
 config\watcom.h include\bytesex.h include\compiler.h include\error.h &
 include\hashtbl.h include\labels.h include\nasm.h include\nasmint.h &
 include\nasmlib.h include\nctype.h include\opflags.h include\perfhash.h &
 include\strlist.h include\tables.h include\warnings.h x86\insnsi.h &
 x86\regs.h
asm\segalloc.$(O): asm\segalloc.c asm\directiv.h asm\pptok.h asm\preproc.h &
 asm\srcfile.h asm\tokens.h config\msvc.h config\unconfig.h config\unknown.h &
 config\watcom.h include\bytesex.h include\compiler.h include\error.h &
 include\hashtbl.h include\iflag.h include\ilog2.h include\insns.h &
 include\labels.h include\nasm.h include\nasmint.h include\nasmlib.h &
 include\nctype.h include\opflags.h include\perfhash.h include\strlist.h &
 include\tables.h include\warnings.h x86\iflaggen.h x86\insnsi.h x86\regs.h
asm\srcfile.$(O): asm\srcfile.c asm\srcfile.h config\msvc.h &
 config\unconfig.h config\unknown.h config\watcom.h include\bytesex.h &
 include\compiler.h include\hashtbl.h include\nasmint.h include\nasmlib.h
asm\stdscan.$(O): asm\stdscan.c asm\directiv.h asm\pptok.h asm\preproc.h &
 asm\quote.h asm\srcfile.h asm\stdscan.h asm\tokens.h config\msvc.h &
 config\unconfig.h config\unknown.h config\watcom.h include\bytesex.h &
 include\compiler.h include\error.h include\hashtbl.h include\iflag.h &
 include\ilog2.h include\insns.h include\labels.h include\nasm.h &
 include\nasmint.h include\nasmlib.h include\nctype.h include\opflags.h &
 include\perfhash.h include\strlist.h include\tables.h include\warnings.h &
 x86\iflaggen.h x86\insnsi.h x86\regs.h
asm\strfunc.$(O): asm\strfunc.c asm\directiv.h asm\pptok.h asm\preproc.h &
 asm\srcfile.h config\msvc.h config\unconfig.h config\unknown.h &
 config\watcom.h include\bytesex.h include\compiler.h include\error.h &
 include\hashtbl.h include\labels.h include\nasm.h include\nasmint.h &
 include\nasmlib.h include\nctype.h include\opflags.h include\perfhash.h &
 include\strlist.h include\tables.h include\warnings.h x86\insnsi.h &
 x86\regs.h
asm\tokhash.$(O): asm\tokhash.c asm\directiv.h asm\pptok.h asm\preproc.h &
 asm\srcfile.h asm\stdscan.h asm\tokens.h config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\error.h include\hashtbl.h include\iflag.h include\ilog2.h &
 include\insns.h include\labels.h include\nasm.h include\nasmint.h &
 include\nasmlib.h include\nctype.h include\opflags.h include\perfhash.h &
 include\strlist.h include\tables.h include\warnings.h x86\iflaggen.h &
 x86\insnsi.h x86\regs.h
asm\warnings.$(O): asm\warnings.c config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\compiler.h include\error.h &
 include\nasmint.h include\warnings.h
common\common.$(O): common\common.c asm\directiv.h asm\pptok.h asm\preproc.h &
 asm\srcfile.h asm\tokens.h config\msvc.h config\unconfig.h config\unknown.h &
 config\watcom.h include\bytesex.h include\compiler.h include\error.h &
 include\hashtbl.h include\iflag.h include\ilog2.h include\insns.h &
 include\labels.h include\nasm.h include\nasmint.h include\nasmlib.h &
 include\nctype.h include\opflags.h include\perfhash.h include\strlist.h &
 include\tables.h include\warnings.h x86\iflaggen.h x86\insnsi.h x86\regs.h
disasm\disasm.$(O): disasm\disasm.c asm\directiv.h asm\pptok.h asm\preproc.h &
 asm\srcfile.h asm\tokens.h config\msvc.h config\unconfig.h config\unknown.h &
 config\watcom.h disasm\disasm.h disasm\sync.h include\bytesex.h &
 include\compiler.h include\disp8.h include\error.h include\hashtbl.h &
 include\iflag.h include\ilog2.h include\insns.h include\labels.h &
 include\nasm.h include\nasmint.h include\nasmlib.h include\nctype.h &
 include\opflags.h include\perfhash.h include\strlist.h include\tables.h &
 include\warnings.h x86\iflaggen.h x86\insnsi.h x86\regdis.h x86\regs.h
disasm\ndisasm.$(O): disasm\ndisasm.c asm\directiv.h asm\pptok.h &
 asm\preproc.h asm\srcfile.h asm\tokens.h config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h disasm\disasm.h disasm\sync.h &
 include\bytesex.h include\compiler.h include\error.h include\hashtbl.h &
 include\iflag.h include\ilog2.h include\insns.h include\labels.h &
 include\nasm.h include\nasmint.h include\nasmlib.h include\nctype.h &
 include\opflags.h include\perfhash.h include\strlist.h include\tables.h &
 include\ver.h include\warnings.h x86\iflaggen.h x86\insnsi.h x86\regs.h
disasm\sync.$(O): disasm\sync.c config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h disasm\sync.h include\bytesex.h &
 include\compiler.h include\nasmint.h include\nasmlib.h
macros\macros.$(O): macros\macros.c asm\directiv.h asm\pptok.h asm\preproc.h &
 asm\srcfile.h config\msvc.h config\unconfig.h config\unknown.h &
 config\watcom.h include\bytesex.h include\compiler.h include\error.h &
 include\hashtbl.h include\labels.h include\nasm.h include\nasmint.h &
 include\nasmlib.h include\nctype.h include\opflags.h include\perfhash.h &
 include\strlist.h include\tables.h include\warnings.h output\outform.h &
 x86\insnsi.h x86\regs.h
nasmlib\alloc.$(O): nasmlib\alloc.c config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\error.h include\nasmint.h include\nasmlib.h include\warnings.h &
 nasmlib\alloc.h
nasmlib\asprintf.$(O): nasmlib\asprintf.c config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\nasmint.h include\nasmlib.h nasmlib\alloc.h
nasmlib\badenum.$(O): nasmlib\badenum.c config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\nasmint.h include\nasmlib.h
nasmlib\bsi.$(O): nasmlib\bsi.c config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\nasmint.h include\nasmlib.h
nasmlib\crc32.$(O): nasmlib\crc32.c config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\hashtbl.h include\nasmint.h include\nasmlib.h
nasmlib\crc64.$(O): nasmlib\crc64.c config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\hashtbl.h include\nasmint.h include\nasmlib.h include\nctype.h
nasmlib\errfile.$(O): nasmlib\errfile.c config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\compiler.h include\nasmint.h
nasmlib\file.$(O): nasmlib\file.c config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\error.h include\nasmint.h include\nasmlib.h include\warnings.h &
 nasmlib\file.h
nasmlib\filename.$(O): nasmlib\filename.c config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\error.h include\nasmint.h include\nasmlib.h include\warnings.h
nasmlib\hashtbl.$(O): nasmlib\hashtbl.c asm\directiv.h asm\pptok.h &
 asm\preproc.h asm\srcfile.h config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\error.h include\hashtbl.h include\labels.h include\nasm.h &
 include\nasmint.h include\nasmlib.h include\nctype.h include\opflags.h &
 include\perfhash.h include\strlist.h include\tables.h include\warnings.h &
 x86\insnsi.h x86\regs.h
nasmlib\ilog2.$(O): nasmlib\ilog2.c config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\compiler.h include\ilog2.h &
 include\nasmint.h
nasmlib\md5c.$(O): nasmlib\md5c.c config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\compiler.h include\md5.h &
 include\nasmint.h
nasmlib\mmap.$(O): nasmlib\mmap.c config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\error.h include\nasmint.h include\nasmlib.h include\warnings.h &
 nasmlib\file.h
nasmlib\nctype.$(O): nasmlib\nctype.c config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\compiler.h include\nasmint.h &
 include\nctype.h
nasmlib\path.$(O): nasmlib\path.c config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\error.h include\nasmint.h include\nasmlib.h include\warnings.h
nasmlib\perfhash.$(O): nasmlib\perfhash.c config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\hashtbl.h include\nasmint.h include\nasmlib.h include\perfhash.h
nasmlib\raa.$(O): nasmlib\raa.c config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\ilog2.h include\nasmint.h include\nasmlib.h include\raa.h
nasmlib\rbtree.$(O): nasmlib\rbtree.c config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\nasmint.h include\nasmlib.h include\rbtree.h
nasmlib\readnum.$(O): nasmlib\readnum.c asm\directiv.h asm\pptok.h &
 asm\preproc.h asm\srcfile.h config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\error.h include\hashtbl.h include\labels.h include\nasm.h &
 include\nasmint.h include\nasmlib.h include\nctype.h include\opflags.h &
 include\perfhash.h include\strlist.h include\tables.h include\warnings.h &
 x86\insnsi.h x86\regs.h
nasmlib\realpath.$(O): nasmlib\realpath.c config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\nasmint.h include\nasmlib.h
nasmlib\rlimit.$(O): nasmlib\rlimit.c config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\nasmint.h include\nasmlib.h
nasmlib\saa.$(O): nasmlib\saa.c config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\nasmint.h include\nasmlib.h include\saa.h
nasmlib\string.$(O): nasmlib\string.c config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\nasmint.h include\nasmlib.h include\nctype.h
nasmlib\strlist.$(O): nasmlib\strlist.c config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\hashtbl.h include\nasmint.h include\nasmlib.h include\strlist.h
nasmlib\ver.$(O): nasmlib\ver.c config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\compiler.h include\nasmint.h &
 include\ver.h version.h
nasmlib\zerobuf.$(O): nasmlib\zerobuf.c config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\nasmint.h include\nasmlib.h
output\codeview.$(O): output\codeview.c asm\directiv.h asm\pptok.h &
 asm\preproc.h asm\srcfile.h config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\error.h include\hashtbl.h include\labels.h include\md5.h &
 include\nasm.h include\nasmint.h include\nasmlib.h include\nctype.h &
 include\opflags.h include\perfhash.h include\rbtree.h include\saa.h &
 include\strlist.h include\tables.h include\warnings.h output\outlib.h &
 output\pecoff.h version.h x86\insnsi.h x86\regs.h
output\legacy.$(O): output\legacy.c asm\directiv.h asm\pptok.h asm\preproc.h &
 asm\srcfile.h config\msvc.h config\unconfig.h config\unknown.h &
 config\watcom.h include\bytesex.h include\compiler.h include\error.h &
 include\hashtbl.h include\labels.h include\nasm.h include\nasmint.h &
 include\nasmlib.h include\nctype.h include\opflags.h include\perfhash.h &
 include\rbtree.h include\saa.h include\strlist.h include\tables.h &
 include\warnings.h output\outlib.h x86\insnsi.h x86\regs.h
output\nulldbg.$(O): output\nulldbg.c asm\directiv.h asm\pptok.h &
 asm\preproc.h asm\srcfile.h config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\error.h include\hashtbl.h include\labels.h include\nasm.h &
 include\nasmint.h include\nasmlib.h include\nctype.h include\opflags.h &
 include\perfhash.h include\rbtree.h include\saa.h include\strlist.h &
 include\tables.h include\warnings.h output\outlib.h x86\insnsi.h x86\regs.h
output\nullout.$(O): output\nullout.c asm\directiv.h asm\pptok.h &
 asm\preproc.h asm\srcfile.h config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\error.h include\hashtbl.h include\labels.h include\nasm.h &
 include\nasmint.h include\nasmlib.h include\nctype.h include\opflags.h &
 include\perfhash.h include\rbtree.h include\saa.h include\strlist.h &
 include\tables.h include\warnings.h output\outlib.h x86\insnsi.h x86\regs.h
output\outaout.$(O): output\outaout.c asm\directiv.h asm\eval.h asm\pptok.h &
 asm\preproc.h asm\srcfile.h asm\stdscan.h config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\error.h include\hashtbl.h include\labels.h include\nasm.h &
 include\nasmint.h include\nasmlib.h include\nctype.h include\opflags.h &
 include\perfhash.h include\raa.h include\rbtree.h include\saa.h &
 include\strlist.h include\tables.h include\warnings.h output\outform.h &
 output\outlib.h x86\insnsi.h x86\regs.h
output\outas86.$(O): output\outas86.c asm\directiv.h asm\pptok.h &
 asm\preproc.h asm\srcfile.h config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\error.h include\hashtbl.h include\labels.h include\nasm.h &
 include\nasmint.h include\nasmlib.h include\nctype.h include\opflags.h &
 include\perfhash.h include\raa.h include\rbtree.h include\saa.h &
 include\strlist.h include\tables.h include\warnings.h output\outform.h &
 output\outlib.h x86\insnsi.h x86\regs.h
output\outbin.$(O): output\outbin.c asm\directiv.h asm\eval.h asm\pptok.h &
 asm\preproc.h asm\srcfile.h asm\stdscan.h config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\error.h include\hashtbl.h include\labels.h include\nasm.h &
 include\nasmint.h include\nasmlib.h include\nctype.h include\opflags.h &
 include\perfhash.h include\rbtree.h include\saa.h include\strlist.h &
 include\tables.h include\warnings.h output\outform.h output\outlib.h &
 x86\insnsi.h x86\regs.h
output\outcoff.$(O): output\outcoff.c asm\directiv.h asm\eval.h asm\pptok.h &
 asm\preproc.h asm\srcfile.h config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\error.h include\hashtbl.h include\ilog2.h include\labels.h &
 include\nasm.h include\nasmint.h include\nasmlib.h include\nctype.h &
 include\opflags.h include\perfhash.h include\raa.h include\rbtree.h &
 include\saa.h include\strlist.h include\tables.h include\ver.h &
 include\warnings.h output\outform.h output\outlib.h output\pecoff.h &
 x86\insnsi.h x86\regs.h
output\outdbg.$(O): output\outdbg.c asm\directiv.h asm\pptok.h asm\preproc.h &
 asm\srcfile.h asm\tokens.h config\msvc.h config\unconfig.h config\unknown.h &
 config\watcom.h include\bytesex.h include\compiler.h include\dbginfo.h &
 include\error.h include\hashtbl.h include\iflag.h include\ilog2.h &
 include\insns.h include\labels.h include\nasm.h include\nasmint.h &
 include\nasmlib.h include\nctype.h include\opflags.h include\perfhash.h &
 include\rbtree.h include\saa.h include\strlist.h include\tables.h &
 include\warnings.h output\outform.h output\outlib.h x86\iflaggen.h &
 x86\insnsi.h x86\regs.h
output\outelf.$(O): output\outelf.c asm\directiv.h asm\eval.h asm\pptok.h &
 asm\preproc.h asm\srcfile.h asm\stdscan.h config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\error.h include\hashtbl.h include\labels.h include\nasm.h &
 include\nasmint.h include\nasmlib.h include\nctype.h include\opflags.h &
 include\perfhash.h include\raa.h include\rbtree.h include\saa.h &
 include\strlist.h include\tables.h include\ver.h include\warnings.h &
 output\dwarf.h output\elf.h output\outelf.h output\outform.h &
 output\outlib.h output\stabs.h x86\insnsi.h x86\regs.h
output\outform.$(O): output\outform.c asm\directiv.h asm\pptok.h &
 asm\preproc.h asm\srcfile.h config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\error.h include\hashtbl.h include\labels.h include\nasm.h &
 include\nasmint.h include\nasmlib.h include\nctype.h include\opflags.h &
 include\perfhash.h include\rbtree.h include\saa.h include\strlist.h &
 include\tables.h include\warnings.h output\outform.h output\outlib.h &
 x86\insnsi.h x86\regs.h
output\outieee.$(O): output\outieee.c asm\directiv.h asm\pptok.h &
 asm\preproc.h asm\srcfile.h config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\error.h include\hashtbl.h include\labels.h include\nasm.h &
 include\nasmint.h include\nasmlib.h include\nctype.h include\opflags.h &
 include\perfhash.h include\rbtree.h include\saa.h include\strlist.h &
 include\tables.h include\ver.h include\warnings.h output\outform.h &
 output\outlib.h x86\insnsi.h x86\regs.h
output\outlib.$(O): output\outlib.c asm\directiv.h asm\pptok.h asm\preproc.h &
 asm\srcfile.h config\msvc.h config\unconfig.h config\unknown.h &
 config\watcom.h include\bytesex.h include\compiler.h include\error.h &
 include\hashtbl.h include\labels.h include\nasm.h include\nasmint.h &
 include\nasmlib.h include\nctype.h include\opflags.h include\perfhash.h &
 include\raa.h include\rbtree.h include\saa.h include\strlist.h &
 include\tables.h include\warnings.h output\outlib.h x86\insnsi.h x86\regs.h
output\outmacho.$(O): output\outmacho.c asm\directiv.h asm\pptok.h &
 asm\preproc.h asm\srcfile.h config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\error.h include\hashtbl.h include\ilog2.h include\labels.h &
 include\nasm.h include\nasmint.h include\nasmlib.h include\nctype.h &
 include\opflags.h include\perfhash.h include\raa.h include\rbtree.h &
 include\saa.h include\strlist.h include\tables.h include\ver.h &
 include\warnings.h output\dwarf.h output\macho.h output\outform.h &
 output\outlib.h x86\insnsi.h x86\regs.h
output\outobj.$(O): output\outobj.c asm\directiv.h asm\eval.h asm\pptok.h &
 asm\preproc.h asm\srcfile.h asm\stdscan.h config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\error.h include\hashtbl.h include\labels.h include\nasm.h &
 include\nasmint.h include\nasmlib.h include\nctype.h include\opflags.h &
 include\perfhash.h include\rbtree.h include\saa.h include\strlist.h &
 include\tables.h include\ver.h include\warnings.h output\outform.h &
 output\outlib.h x86\insnsi.h x86\regs.h
stdlib\snprintf.$(O): stdlib\snprintf.c config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\nasmint.h include\nasmlib.h
stdlib\strlcpy.$(O): stdlib\strlcpy.c config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\compiler.h include\nasmint.h
stdlib\strnlen.$(O): stdlib\strnlen.c config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\compiler.h include\nasmint.h
stdlib\strrchrnul.$(O): stdlib\strrchrnul.c config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\compiler.h include\nasmint.h
stdlib\vsnprintf.$(O): stdlib\vsnprintf.c config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\bytesex.h include\compiler.h &
 include\error.h include\nasmint.h include\nasmlib.h include\warnings.h
x86\disp8.$(O): x86\disp8.c asm\directiv.h asm\pptok.h asm\preproc.h &
 asm\srcfile.h config\msvc.h config\unconfig.h config\unknown.h &
 config\watcom.h include\bytesex.h include\compiler.h include\disp8.h &
 include\error.h include\hashtbl.h include\labels.h include\nasm.h &
 include\nasmint.h include\nasmlib.h include\nctype.h include\opflags.h &
 include\perfhash.h include\strlist.h include\tables.h include\warnings.h &
 x86\insnsi.h x86\regs.h
x86\iflag.$(O): x86\iflag.c config\msvc.h config\unconfig.h config\unknown.h &
 config\watcom.h include\compiler.h include\iflag.h include\ilog2.h &
 include\nasmint.h x86\iflaggen.h
x86\insnsa.$(O): x86\insnsa.c asm\directiv.h asm\pptok.h asm\preproc.h &
 asm\srcfile.h asm\tokens.h config\msvc.h config\unconfig.h config\unknown.h &
 config\watcom.h include\bytesex.h include\compiler.h include\error.h &
 include\hashtbl.h include\iflag.h include\ilog2.h include\insns.h &
 include\labels.h include\nasm.h include\nasmint.h include\nasmlib.h &
 include\nctype.h include\opflags.h include\perfhash.h include\strlist.h &
 include\tables.h include\warnings.h x86\iflaggen.h x86\insnsi.h x86\regs.h
x86\insnsb.$(O): x86\insnsb.c asm\directiv.h asm\pptok.h asm\preproc.h &
 asm\srcfile.h asm\tokens.h config\msvc.h config\unconfig.h config\unknown.h &
 config\watcom.h include\bytesex.h include\compiler.h include\error.h &
 include\hashtbl.h include\iflag.h include\ilog2.h include\insns.h &
 include\labels.h include\nasm.h include\nasmint.h include\nasmlib.h &
 include\nctype.h include\opflags.h include\perfhash.h include\strlist.h &
 include\tables.h include\warnings.h x86\iflaggen.h x86\insnsi.h x86\regs.h
x86\insnsd.$(O): x86\insnsd.c asm\directiv.h asm\pptok.h asm\preproc.h &
 asm\srcfile.h asm\tokens.h config\msvc.h config\unconfig.h config\unknown.h &
 config\watcom.h include\bytesex.h include\compiler.h include\error.h &
 include\hashtbl.h include\iflag.h include\ilog2.h include\insns.h &
 include\labels.h include\nasm.h include\nasmint.h include\nasmlib.h &
 include\nctype.h include\opflags.h include\perfhash.h include\strlist.h &
 include\tables.h include\warnings.h x86\iflaggen.h x86\insnsi.h x86\regs.h
x86\insnsn.$(O): x86\insnsn.c config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\compiler.h include\nasmint.h &
 include\tables.h x86\insnsi.h
x86\regdis.$(O): x86\regdis.c x86\regdis.h x86\regs.h
x86\regflags.$(O): x86\regflags.c asm\directiv.h asm\pptok.h asm\preproc.h &
 asm\srcfile.h config\msvc.h config\unconfig.h config\unknown.h &
 config\watcom.h include\bytesex.h include\compiler.h include\error.h &
 include\hashtbl.h include\labels.h include\nasm.h include\nasmint.h &
 include\nasmlib.h include\nctype.h include\opflags.h include\perfhash.h &
 include\strlist.h include\tables.h include\warnings.h x86\insnsi.h &
 x86\regs.h
x86\regs.$(O): x86\regs.c config\msvc.h config\unconfig.h config\unknown.h &
 config\watcom.h include\compiler.h include\nasmint.h include\tables.h &
 x86\insnsi.h
x86\regvals.$(O): x86\regvals.c config\msvc.h config\unconfig.h &
 config\unknown.h config\watcom.h include\compiler.h include\nasmint.h &
 include\tables.h x86\insnsi.h
