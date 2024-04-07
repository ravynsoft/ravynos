#
# Makefile to build perl on Windows using Microsoft NMAKE.
# Supported compilers:
#	Microsoft Visual C++ 12.0 or later
#
# This is set up to build a perl.exe that runs off a shared library
# (perl538.dll).  Also makes individual DLLs for the XS extensions.
#

##
## Make sure you read README.win32 *before* you mess with anything here!
##

##
## Build configuration.  Edit the values below to suit your needs.
##

#
# Set these to wherever you want "nmake install" to put your
# newly built perl.
#
INST_DRV	= c:
INST_TOP	= $(INST_DRV)\perl

#
# Uncomment if you want to build a 32-bit Perl using a 32-bit compiler
# on a 64-bit version of Windows.
#
#WIN64		= undef

#
# Comment this out if you DON'T want your perl installation to be versioned.
# This means that the new installation will overwrite any files from the
# old installation at the same INST_TOP location.  Leaving it enabled is
# the safest route, as perl adds the extra version directory to all the
# locations it installs files to.  If you disable it, an alternative
# versioned installation can be obtained by setting INST_TOP above to a
# path that includes an arbitrary version string.
#
#INST_VER	= \5.38.2

#
# Comment this out if you DON'T want your perl installation to have
# architecture specific components.  This means that architecture-
# specific files will be installed along with the architecture-neutral
# files.  Leaving it enabled is safer and more flexible, in case you
# want to build multiple flavors of perl and install them together in
# the same location.  Commenting it out gives you a simpler
# installation that is easier to understand for beginners.
#
#INST_ARCH	= \$(ARCHNAME)

#
# Uncomment this if you want perl to run
# 	$Config{sitelibexp}\sitecustomize.pl
# before anything else.  This script can then be set up, for example,
# to add additional entries to @INC.
#
#USE_SITECUST	= define

#
# uncomment to enable multiple interpreters.  This is needed for fork()
# emulation and for thread support, and is auto-enabled by USE_IMP_SYS
# and USE_ITHREADS below.
#
USE_MULTI	= define

#
# Interpreter cloning/threads; now reasonably complete.
# This should be enabled to get the fork() emulation.  This needs (and
# will auto-enable) USE_MULTI above.
#
USE_ITHREADS	= define

#
# uncomment to enable the implicit "host" layer for all system calls
# made by perl.  This is also needed to get fork().  This needs (and
# will auto-enable) USE_MULTI above.
#
USE_IMP_SYS	= define

#
# Uncomment this if you're building a 32-bit perl and want 64-bit integers.
# (If you're building a 64-bit perl then you will have 64-bit integers whether
# or not this is uncommented.)
#
#USE_64_BIT_INT	= define

#
# Comment this out if you want the legacy default behavior of including '.' at
# the end of @INC.
#
DEFAULT_INC_EXCLUDES_DOT = define

#
# Uncomment this if you want to disable looking up values from
# HKEY_CURRENT_USER\Software\Perl and HKEY_LOCAL_MACHINE\Software\Perl in
# the Registry.
#
#USE_NO_REGISTRY = define

#
# uncomment exactly one of the following
#
# Visual C++ 2013 (aka Visual C++ 12.0) (full version or Express Edition)
#CCTYPE		= MSVC120
# Visual C++ 2015 (aka Visual C++ 14.0) (full version or Express Edition)
#CCTYPE		= MSVC140
# Visual C++ 2017 (aka Visual C++ 14.1) (full version or Community Edition)
#CCTYPE		= MSVC141
# Visual C++ 2019 (aka Visual C++ 14.2) (full version or Community Edition)
#CCTYPE		= MSVC142
# Visual C++ 2022 (aka Visual C++ 14.3) (full version or Community Edition)
#CCTYPE		= MSVC143

#
# If you are using Intel C++ Compiler uncomment this
#
#__ICC		= define

#
# Uncomment this if you want to build everything in C++ mode
# This requires at least MSVC 2019.
#
#USE_CPLUSPLUS	= define

#
# uncomment next line if you want debug version of perl (big/slow)
# If not enabled, we automatically try to use maximum optimization
# with all compilers that are known to have a working optimizer.
#
# You can also set CFG = DebugSymbols for a slightly smaller/faster
# debug build without the special debugging code in perl which is
# enabled via -DDEBUGGING;
#
# or you can set CFG = DebugFull for an even fuller (bigger/slower)
# debug build using the debug version of the CRT, and enabling VC++
# debug features such as extra assertions and invalid parameter warnings
# in perl and CRT code via -D_DEBUG.  (Note that the invalid parameter
# handler does get triggered from time to time in this configuration,
# which causes warnings to be printed on STDERR, which in turn causes a
# few tests to fail.)
#
#CFG		= Debug

#
# uncomment to enable linking with setargv.obj under the Visual C
# compiler. Setting this options enables perl to expand wildcards in
# arguments, but it may be harder to use alternate methods like
# File::DosGlob that are more powerful.  This option is supported only with
# Visual C.
#
#USE_SETARGV	= define

#
# set this if you wish to use perl's malloc
# WARNING: Turning this on/off WILL break binary compatibility with extensions
# you may have compiled with/without it.  Be prepared to recompile all
# extensions if you change the default.  Currently, this cannot be enabled
# if you ask for USE_IMP_SYS above.
#
#PERL_MALLOC	= define

#
# set this to enable debugging mstats
# This must be enabled to use the Devel::Peek::mstat() function.  This cannot
# be enabled without PERL_MALLOC as well.
#
#DEBUG_MSTATS	= define

#
# set this to additionally provide a statically linked perl-static.exe.
# Note that dynamic loading will not work with this perl, so you must
# include required modules statically using the STATIC_EXT or ALL_STATIC
# variables below. A static library perl538s.lib will also be created.
# Ordinary perl.exe is not affected by this option.
#
#BUILD_STATIC	= define

#
# in addition to BUILD_STATIC the option ALL_STATIC makes *every*
# extension get statically built.
# This will result in a very large perl executable, but the main purpose
# is to have proper linking set so as to be able to create miscellaneous
# executables with different built-in extensions.
#
#ALL_STATIC	= define

#
# set the install location of the compiler
# Running VCVARS32.BAT, VCVARSALL.BAT or similar is *required* when using
# Visual C++.
# Versions of Visual C++ up to VC++ 14.0 define $(VCINSTALLDIR), but from
# VC++ 14.1 we need the subfolder given by $(VCToolsInstallDir).
#
!IF "$(CCTYPE)" == "MSVC120" || "$(CCTYPE)" == "MSVC140"
CCHOME		= $(VCINSTALLDIR)
!ELSE
CCHOME		= $(VCTOOLSINSTALLDIR)
!ENDIF

#
# Additional compiler flags can be specified here.
#
BUILDOPT	= $(BUILDOPTEXTRA)

#
# This should normally be disabled.  Enabling it will disable the File::Glob
# implementation of CORE::glob.
#
#BUILDOPT	= $(BUILDOPT) -DPERL_EXTERNAL_GLOB

#
# Perl needs to read scripts in text mode so that the DATA filehandle
# works correctly with seek() and tell(), or around auto-flushes of
# all filehandles (e.g. by system(), backticks, fork(), etc).
#
# The current version on the ByteLoader module on CPAN however only
# works if scripts are read in binary mode.  But before you disable text
# mode script reading (and break some DATA filehandle functionality)
# please check first if an updated ByteLoader isn't available on CPAN.
#
BUILDOPT	= $(BUILDOPT) -DPERL_TEXTMODE_SCRIPTS

#
# specify semicolon-separated list of extra directories that modules will
# look for libraries (spaces in path names need not be quoted)
#
EXTRALIBDIRS	=

#
# set this to your email address (perl will guess a value from
# your loginname and your hostname, which may not be right)
#
#EMAIL		=

##
## Build configuration ends.
##

##################### CHANGE THESE ONLY IF YOU MUST #####################

!IF "$(USE_IMP_SYS)" == "define"
PERL_MALLOC	= undef
DEBUG_MSTATS	= undef
!ENDIF

!IF "$(PERL_MALLOC)" == ""
PERL_MALLOC	= undef
DEBUG_MSTATS	= undef
!ENDIF

!IF "$(DEBUG_MSTATS)" == ""
DEBUG_MSTATS	= undef
!ENDIF

!IF "$(DEBUG_MSTATS)" == "define"
BUILDOPT	= $(BUILDOPT) -DPERL_DEBUGGING_MSTATS
!ENDIF

!IF "$(USE_SITECUST)" == ""
USE_SITECUST	= undef
!ENDIF

!IF "$(USE_MULTI)" == ""
USE_MULTI	= undef
!ENDIF

!IF "$(USE_ITHREADS)" == ""
USE_ITHREADS	= undef
!ENDIF

!IF "$(USE_IMP_SYS)" == ""
USE_IMP_SYS	= undef
!ENDIF

!IF "$(USE_64_BIT_INT)" == ""
USE_64_BIT_INT	= undef
!ENDIF

!IF "$(DEFAULT_INC_EXCLUDES_DOT)" == ""
DEFAULT_INC_EXCLUDES_DOT = undef
!ENDIF

!IF "$(USE_NO_REGISTRY)" == ""
USE_NO_REGISTRY	= undef
!ENDIF

!IF "$(USE_IMP_SYS)$(USE_MULTI)" == "defineundef"
USE_MULTI	= define
!ENDIF

!IF "$(USE_ITHREADS)$(USE_MULTI)" == "defineundef"
USE_MULTI	= define
!ENDIF

!IF "$(USE_SITECUST)" == "define"
BUILDOPT	= $(BUILDOPT) -DUSE_SITECUSTOMIZE
!ENDIF

!IF "$(USE_MULTI)" != "undef"
BUILDOPT	= $(BUILDOPT) -DMULTIPLICITY
!ENDIF

!IF "$(USE_IMP_SYS)" != "undef"
BUILDOPT	= $(BUILDOPT) -DPERL_IMPLICIT_SYS
!ENDIF

!IF "$(USE_NO_REGISTRY)" != "undef"
BUILDOPT	= $(BUILDOPT) -DWIN32_NO_REGISTRY
!ENDIF

!IF "$(PROCESSOR_ARCHITECTURE)" == ""
PROCESSOR_ARCHITECTURE	= x86
!ENDIF

!IF "$(WIN64)" == ""
# When we are running from a 32bit cmd.exe on AMD64 then
# PROCESSOR_ARCHITECTURE is set to x86 and PROCESSOR_ARCHITEW6432
# is set to AMD64
!  IF "$(PROCESSOR_ARCHITEW6432)" != ""
PROCESSOR_ARCHITECTURE	= $(PROCESSOR_ARCHITEW6432)
WIN64			= define
!  ELSEIF "$(PROCESSOR_ARCHITECTURE)" == "AMD64" || "$(PROCESSOR_ARCHITECTURE)" == "IA64" || "$(PROCESSOR_ARCHITECTURE)" == "ARM64"
WIN64			= define
!  ELSE
WIN64			= undef
!  ENDIF
!ENDIF

!IF "$(WIN64)" == "define"
USE_64_BIT_INT	= define
!ENDIF

ARCHITECTURE = $(PROCESSOR_ARCHITECTURE)

!IF "$(ARCHITECTURE)" == "AMD64"
ARCHITECTURE	= x64
!ELSEIF "$(ARCHITECTURE)" == "IA64"
ARCHITECTURE	= ia64
!ELSEIF "$(ARCHITECTURE)" == "ARM64"
ARCHITECTURE	= arm64
!ENDIF

!IF "$(USE_MULTI)" == "define"
ARCHNAME	= MSWin32-$(ARCHITECTURE)-multi
!ELSE
ARCHNAME	= MSWin32-$(ARCHITECTURE)-perlio
!ENDIF

!IF "$(USE_ITHREADS)" == "define"
ARCHNAME	= $(ARCHNAME)-thread
!ENDIF

!IF "$(WIN64)" != "define" && "$(USE_64_BIT_INT)" == "define"
ARCHNAME	= $(ARCHNAME)-64int
!ENDIF

# Loading DLLs on demand makes the test suite run in about 10% less time.
# If no registry, advapi32 is only used for Perl_pp_getlogin/getlogin/GetUserNameA
# which is rare to execute
!IF "$(USE_NO_REGISTRY)" != "undef"
DELAYLOAD	= -DELAYLOAD:ws2_32.dll -DELAYLOAD:advapi32.dll delayimp.lib
MINIDELAYLOAD	=
!ELSE
DELAYLOAD	= -DELAYLOAD:ws2_32.dll delayimp.lib
#miniperl never does any registry lookups
MINIDELAYLOAD	= -DELAYLOAD:advapi32.dll
!ENDIF

# Set the install location of the compiler headers/libraries.
# These are saved into $Config{incpath} and $Config{libpth}.
CCINCDIR	= $(CCHOME)\include
!IF "$(CCTYPE)" == "MSVC120" || "$(CCTYPE)" == "MSVC140"
!  IF "$(WIN64)" == "define"
CCLIBDIR	= $(CCHOME)\lib\amd64
!  ELSE
CCLIBDIR	= $(CCHOME)\lib
!  ENDIF
!ELSE
!  IF "$(ARCHITECTURE)" == "arm64"
CCLIBDIR	= $(CCHOME)\lib\arm64
!  ELSEIF "$(WIN64)" == "define"
CCLIBDIR	= $(CCHOME)\lib\x64
!  ELSE
CCLIBDIR	= $(CCHOME)\lib\x86
!  ENDIF
!ENDIF

ARCHDIR		= ..\lib\$(ARCHNAME)
COREDIR		= ..\lib\CORE
AUTODIR		= ..\lib\auto
LIBDIR		= ..\lib
EXTDIR		= ..\ext
DISTDIR		= ..\dist
CPANDIR		= ..\cpan
PODDIR		= ..\pod
HTMLDIR		= .\html

INST_SCRIPT	= $(INST_TOP)$(INST_VER)\bin
INST_BIN	= $(INST_SCRIPT)$(INST_ARCH)
INST_LIB	= $(INST_TOP)$(INST_VER)\lib
INST_ARCHLIB	= $(INST_LIB)$(INST_ARCH)
INST_COREDIR	= $(INST_ARCHLIB)\CORE
INST_HTML	= $(INST_TOP)$(INST_VER)\html

#
# Programs to compile, build .lib files and link
#

# Poison the recipe if no CCTYPE is set, and delete mini\.exists so everything
# is rebuilt next time to avoid reusing a mini config.h written with no sane CC
# version set
!IF "$(CCTYPE)" == ""
CC		= @echo CCTYPE makefile variable not set && del mini\.exists && exit 1 &&
LINK32		= @echo CCTYPE makefile variable not set && del mini\.exists && exit 1 &&
!ELSEIF "$(__ICC)" != "define"
CC		= cl
LINK32		= link
!ELSE
CC		= icl
LINK32		= xilink
!ENDIF
LIB32		= $(LINK32) -lib
RSC		= rc

#
# Options
#

INCLUDES	= -I$(COREDIR) -I.\include -I. -I..
#PCHFLAGS	= -Fpc:\temp\vcmoduls.pch -YX
DEFINES		= -DWIN32 -D_CONSOLE -DNO_STRICT
LOCDEFS		= -DPERLDLL -DPERL_CORE
CXX_FLAG	= -TP -EHsc
!IF "$(USE_CPLUSPLUS)" == "define"
CXX_FLAG	= $(CXX_FLAG) -std:c++20
!ENDIF
EXTRACFLAGS	= -nologo -GF -W3

!IF "$(CCTYPE)" == "MSVC120"
LIBC		= msvcrt.lib
!ELSE
LIBC		= ucrt.lib
!ENDIF

!IF  "$(CFG)" == "Debug"
OPTIMIZE	= -Od -Zi
LINK_DBG	= -debug
DEFINES		= $(DEFINES) -DDEBUGGING
EXTRACFLAGS	= $(EXTRACFLAGS) -MD
!ELSEIF  "$(CFG)" == "DebugSymbols"
OPTIMIZE	= -Od -Zi
LINK_DBG	= -debug
EXTRACFLAGS	= $(EXTRACFLAGS) -MD
!ELSEIF  "$(CFG)" == "DebugFull"
!  IF "$(CCTYPE)" == "MSVC120"
LIBC		= msvcrtd.lib
!  ELSE
LIBC		= ucrtd.lib
!  ENDIF
OPTIMIZE	= -Od -Zi
LINK_DBG	= -debug
DEFINES		= $(DEFINES) -D_DEBUG -DDEBUGGING
EXTRACFLAGS	= $(EXTRACFLAGS) -MDd
!ELSE
# Enable Whole Program Optimizations (WPO) and Link Time Code Generation (LTCG).
# -O1 yields smaller code, which turns out to be faster than -O2 on x86 and x64
OPTIMIZE	= -O1 -Zi -GL
# we enable debug symbols in release builds also
LINK_DBG	= -debug -opt:ref,icf -ltcg
# you may want to enable this if you want COFF symbols in the executables
# in addition to the PDB symbols.  The default Dr. Watson that ships with
# Windows can use the former but not latter.  The free WinDbg can be
# installed to get better stack traces from just the PDB symbols, so we
# avoid the bloat of COFF symbols by default.
#LINK_DBG	= $(LINK_DBG) -debugtype:both
LIB_FLAGS	= -ltcg
EXTRACFLAGS	= $(EXTRACFLAGS) -MD
!ENDIF

!IF "$(WIN64)" == "define"
DEFINES		= $(DEFINES) -DWIN64
OPTIMIZE	= $(OPTIMIZE) -fp:precise
!ENDIF

# For now, silence warnings about "unsafe" CRT functions
# and POSIX CRT function names being deprecated.
DEFINES		= $(DEFINES) -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE

# Likewise for deprecated Winsock APIs in VC++ 14.0 onwards for now.
!IF "$(CCTYPE)" != "MSVC120"
DEFINES		= $(DEFINES) -D_WINSOCK_DEPRECATED_NO_WARNINGS
!ENDIF

LIBBASEFILES	= \
		oldnames.lib kernel32.lib user32.lib gdi32.lib winspool.lib \
		comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib \
		netapi32.lib uuid.lib ws2_32.lib mpr.lib winmm.lib \
		version.lib odbc32.lib odbccp32.lib comctl32.lib

!IF "$(CCTYPE)" != "MSVC120"
!  IF  "$(CFG)" == "DebugFull"
LIBBASEFILES	= $(LIBBASEFILES) msvcrtd.lib vcruntimed.lib
!  ELSE
LIBBASEFILES	= $(LIBBASEFILES) msvcrt.lib vcruntime.lib
!  ENDIF
!ENDIF

# Avoid __intel_new_proc_init link error for libircmt.
# libmmd is /MD equivalent, other variants exist.
# libmmd is Intel C's math addon funcs to MS CRT, contains long doubles, C99,
# and optimized C89 funcs
!IF "$(__ICC)" == "define"
LIBBASEFILES	= $(LIBBASEFILES) libircmt.lib libmmd.lib
!ENDIF

LIBFILES	= $(LIBBASEFILES) $(LIBC)

!IF "$(__ICC)" == "define"
EXTRACFLAGS	= $(EXTRACFLAGS) -Qstd=c99
!ENDIF
!IF "$(USE_CPLUSPLUS)" == "define"
EXTRACFLAGS	= $(EXTRACFLAGS) $(CXX_FLAG)
!ENDIF
CFLAGS		= $(EXTRACFLAGS) $(INCLUDES) $(DEFINES) $(LOCDEFS) \
		$(PCHFLAGS) $(OPTIMIZE)
LINK_FLAGS	= -nologo -nodefaultlib $(LINK_DBG) \
		-libpath:"$(INST_COREDIR)" \
		-machine:$(PROCESSOR_ARCHITECTURE)
LIB_FLAGS	= $(LIB_FLAGS) -nologo
OBJOUT_FLAG	= -Fo
EXEOUT_FLAG	= -Fe

CFLAGS_O	= $(CFLAGS) $(BUILDOPT)

RSC_FLAGS	=

# VS 2017 (VC++ 14.1) requires at minimum Windows 7 SP1 (with latest Windows Updates)

# For XP support in >= VS 2013 (VC++ 12.0), subsystem is always in Config.pm
# LINK_FLAGS.
!IF "$(WIN64)" == "define"
LINK_FLAGS	= $(LINK_FLAGS) -subsystem:console,"5.02"
!ELSE
LINK_FLAGS	= $(LINK_FLAGS) -subsystem:console,"5.01"
!ENDIF

BLINK_FLAGS	= $(PRIV_LINK_FLAGS) $(LINK_FLAGS)

#################### do not edit below this line #######################
############# NO USER-SERVICEABLE PARTS BEYOND THIS POINT ##############

o = .obj

#
# Rules
#

#clear the list, we dont support .cxx .bas .cbl .for .pas .f .f90
# .asm .cpp are not currently used but they are included for completeness
.SUFFIXES :
.SUFFIXES : .c $(o) .cpp .asm .dll .lib .exe .rc .res

.c$(o):
	$(CC) -c -I$(<D) $(CFLAGS_O) $(OBJOUT_FLAG)$@ $<

.c.i:
	$(CC) -c -I$(<D) $(CFLAGS_O) -P $(OBJOUT_FLAG)$@ $<

.y.c:
	$(NOOP)

$(o).dll:
	$(LINK32) -dll -implib:$(*B).lib -def:$(*B).def \
	    -out:$@ $(BLINK_FLAGS) $(LIBFILES) $< $(LIBPERL)

.rc.res:
	$(RSC) -i.. $(RSC_FLAGS) $<

#
# various targets

# makedef.pl must be updated if this changes, and this should normally
# only change when there is an incompatible revision of the public API.
PERLIMPLIB	= ..\perl538.lib
PERLSTATICLIB	= ..\perl538s.lib
PERLDLL		= ..\perl538.dll

MINIPERL	= ..\miniperl.exe
MINIDIR		= .\mini
PERLEXE		= ..\perl.exe
WPERLEXE	= ..\wperl.exe
PERLEXESTATIC	= ..\perl-static.exe
GLOBEXE		= ..\perlglob.exe
CONFIGPM	= ..\lib\Config.pm ..\lib\Config_heavy.pl
GENUUDMAP	= ..\generate_uudmap.exe
!IF "$(BUILD_STATIC)" == "define" || "$(ALL_STATIC)" == "define"
PERLSTATIC	= static
!ELSE
PERLSTATIC	= 
!ENDIF

# Unicode data files generated by mktables
FIRSTUNIFILE     = ..\lib\unicore\Decomposition.pl
UNIDATAFILES	 = ..\lib\unicore\Decomposition.pl \
		   ..\lib\unicore\CombiningClass.pl ..\lib\unicore\Name.pl \
		   ..\lib\unicore\Heavy.pl ..\lib\unicore\mktables.lst     \
		   ..\lib\unicore\UCD.pl ..\lib\unicore\Name.pm            \
		   ..\lib\unicore\TestProp.pl

# Directories of Unicode data files generated by mktables
UNIDATADIR1	= ..\lib\unicore\To
UNIDATADIR2	= ..\lib\unicore\lib

PERLEXE_MANIFEST= .\perlexe.manifest
PERLEXE_ICO	= .\perlexe.ico
PERLEXE_RES	= .\perlexe.res
PERLDLL_RES	=

# Nominate a target which causes extensions to be re-built
# This used to be $(PERLEXE), but at worst it is the .dll that they depend
# on and really only the interface - i.e. the .def file used to export symbols
# from the .dll
PERLDEP		= perldll.def

PL2BAT		= bin\pl2bat.pl
GLOBBAT		= bin\perlglob.bat

UTILS		=			\
		..\utils\h2ph		\
		..\utils\splain		\
		..\utils\perlbug	\
		..\utils\pl2pm 		\
		..\utils\h2xs		\
		..\utils\perldoc	\
		..\utils\perlivp	\
		..\utils\libnetcfg	\
		..\utils\enc2xs		\
		..\utils\encguess		\
		..\utils\piconv		\
		..\utils\corelist	\
		..\utils\cpan		\
		..\utils\xsubpp		\
		..\utils\prove		\
		..\utils\ptar		\
		..\utils\ptardiff	\
		..\utils\ptargrep	\
		..\utils\zipdetails	\
		..\utils\shasum		\
		..\utils\instmodsh	\
		..\utils\json_pp	\
		..\utils\pod2html	\
		..\utils\streamzip	\
		bin\exetype.pl		\
		bin\runperl.pl		\
		bin\pl2bat.pl		\
		bin\perlglob.pl		\
		bin\search.pl

MAKE		= nmake -nologo
MAKE_BARE	= nmake

CFGSH_TMPL	= config.vc
CFGH_TMPL	= config_H.vc
INT64		= __int64

XCOPY		= xcopy /f /r /i /d /y
RCOPY		= xcopy /f /r /i /e /d /y
NOOP		= @rem
NULL		=

DEL		= del

MICROCORE_SRC	=		\
		..\av.c		\
		..\caretx.c	\
		..\class.c	\
		..\builtin.c	\
		..\deb.c	\
		..\doio.c	\
		..\doop.c	\
		..\dquote.c	\
		..\dump.c	\
		..\globals.c	\
		..\gv.c		\
		..\mro_core.c	\
		..\hv.c		\
		..\locale.c	\
		..\keywords.c	\
		..\mathoms.c    \
		..\mg.c		\
		..\numeric.c	\
		..\op.c		\
		..\pad.c	\
		..\peep.c	\
		..\perl.c	\
		..\perly.c	\
		..\pp.c		\
		..\pp_ctl.c	\
		..\pp_hot.c	\
		..\pp_pack.c	\
		..\pp_sort.c	\
		..\pp_sys.c	\
		..\reentr.c	\
		..\regcomp.c	\
		..\regcomp_trie.c	\
		..\regcomp_debug.c	\
		..\regcomp_invlist.c	\
		..\regcomp_study.c	\
		..\regexec.c	\
		..\run.c	\
		..\scope.c	\
		..\sv.c		\
		..\taint.c	\
		..\time64.c	\
		..\toke.c	\
		..\universal.c	\
		..\utf8.c	\
		..\util.c

EXTRACORE_SRC	= $(EXTRACORE_SRC) perllib.c

!IF "$(PERL_MALLOC)" == "define"
EXTRACORE_SRC	= $(EXTRACORE_SRC) ..\malloc.c
!ENDIF

EXTRACORE_SRC	= $(EXTRACORE_SRC) ..\perlio.c

WIN32_SRC	=		\
		.\win32.c	\
		.\win32sck.c	\
		.\win32thread.c	\
		.\fcrypt.c

CORE_NOCFG_H	=		\
		..\av.h		\
		..\cop.h	\
		..\cv.h		\
		..\dosish.h	\
		..\embed.h	\
		..\form.h	\
		..\gv.h		\
		..\handy.h	\
		..\hv.h		\
		..\hv_func.h	\
		..\iperlsys.h	\
		..\mg.h		\
		..\nostdio.h	\
		..\op.h		\
		..\opcode.h	\
		..\perl.h	\
		..\perlapi.h	\
		..\perlsdio.h	\
		..\perly.h	\
		..\pp.h		\
		..\proto.h	\
		..\regcomp.h	\
		..\regcomp_internal.h	\
		..\regexp.h	\
		..\scope.h	\
		..\sv.h		\
		..\thread.h	\
		..\unixish.h	\
		..\utf8.h	\
		..\util.h	\
		..\warnings.h	\
		..\XSUB.h	\
		..\EXTERN.h	\
		..\perlvars.h	\
		..\intrpvar.h	\
		.\include\dirent.h	\
		.\include\netdb.h	\
		.\include\sys\errno2.h	\
		.\include\sys\socket.h	\
		.\win32.h

CORE_H		= $(CORE_NOCFG_H) .\config.h ..\git_version.h

UUDMAP_H	= ..\uudmap.h
BITCOUNT_H	= ..\bitcount.h
MG_DATA_H	= ..\mg_data.h
GENERATED_HEADERS = $(UUDMAP_H) $(BITCOUNT_H) $(MG_DATA_H)

MICROCORE_OBJ	= $(MICROCORE_SRC:.c=.obj)
CORE_OBJ	= $(MICROCORE_OBJ) $(EXTRACORE_SRC:.c=.obj)
WIN32_OBJ	= $(WIN32_SRC:.c=.obj)
MINICORE_OBJ	= $(MICROCORE_OBJ:..\=.\mini\)	\
		  $(MINIDIR)\miniperlmain$(o)	\
		  $(MINIDIR)\perlio$(o)
MINIWIN32_OBJ	= $(WIN32_OBJ:.\=.\mini\)
MINI_OBJ	= $(MINICORE_OBJ) $(MINIWIN32_OBJ)
DLL_OBJ		= $(DYNALOADER)
GENUUDMAP_OBJ	= $(GENUUDMAP:.exe=.obj)

PERLDLL_OBJ	= $(CORE_OBJ)
PERLEXE_OBJ	= perlmain$(o)
PERLEXEST_OBJ	= perlmainst$(o)

PERLDLL_OBJ	= $(PERLDLL_OBJ) $(WIN32_OBJ) $(DLL_OBJ)

!IF "$(USE_SETARGV)" != ""
SETARGV_OBJ	= setargv$(o)
!ENDIF

!IF "$(ALL_STATIC)" == "define"
# some exclusions, unfortunately, until fixed:
#  - MakeMaker isn't capable enough for SDBM_File (small bug)
STATIC_EXT	= * !SDBM_File
!ELSE
# specify static extensions here, for example:
# (be sure to include Win32CORE to load Win32 on demand)
#STATIC_EXT	= Win32CORE Cwd Compress/Raw/Zlib
STATIC_EXT	= Win32CORE
!ENDIF

DYNALOADER	= ..\DynaLoader$(o)

CFG_VARS	=					\
		"INST_TOP=$(INST_TOP)"			\
		"INST_VER=$(INST_VER)"			\
		"INST_ARCH=$(INST_ARCH)"		\
		"archname=$(ARCHNAME)"			\
		"cc=$(CC)"				\
		"ld=$(LINK32)"				\
		"ccflags=$(EXTRACFLAGS) $(DEFINES) $(BUILDOPT)"	\
		"usecplusplus=$(USE_CPLUSPLUS)"		\
		"cf_email=$(EMAIL)"	 		\
		"d_mymalloc=$(PERL_MALLOC)"		\
		"libs=$(LIBFILES)"			\
		"incpath=$(CCINCDIR:"=\")"		\
		"libperl=$(PERLIMPLIB:..\=)"		\
		"libpth=$(CCLIBDIR:"=\");$(EXTRALIBDIRS:"=\")"	\
		"libc=$(LIBC)"				\
		"make=$(MAKE_BARE)"				\
		"static_ext=$(STATIC_EXT)"		\
		"usethreads=$(USE_ITHREADS)"		\
		"useithreads=$(USE_ITHREADS)"		\
		"usemultiplicity=$(USE_MULTI)"		\
		"use64bitint=$(USE_64_BIT_INT)"		\
		"uselongdouble=undef"			\
		"usequadmath=undef"			    \
		"usesitecustomize=$(USE_SITECUST)"	\
		"default_inc_excludes_dot=$(DEFAULT_INC_EXCLUDES_DOT)"	\
		"LINK_FLAGS=$(LINK_FLAGS:"=\")"		\
		"optimize=$(OPTIMIZE:"=\")"		\
		"WIN64=$(WIN64)"                        \
		"SKIP_CCHOME_CHECK=$(SKIP_CCHOME_CHECK)"

#
# Top targets
#

all : ..\git_version.h $(GLOBEXE) $(CONFIGPM) \
	$(UNIDATAFILES) MakePPPort $(PERLEXE) Extensions_nonxs Extensions \
	$(PERLSTATIC)
	@echo	Everything is up to date. '$(MAKE_BARE) test' to run test suite.

regnodes : ..\regnodes.h

..\regcomp$(o) : ..\regcomp.h ..\regcomp_internal.h ..\regnodes.h ..\regcharclass.h

..\regcomp_debug$(o) : ..\regcomp.h ..\regcomp_internal.h ..\regnodes.h ..\regcharclass.h

..\regcomp_invlist$(o) : ..\regcomp.h ..\regcomp_internal.h ..\regnodes.h ..\regcharclass.h

..\regcomp_study$(o) : ..\regcomp.h ..\regcomp_internal.h ..\regnodes.h ..\regcharclass.h

..\regcomp_trie$(o) : ..\regcomp.h ..\regcomp_internal.h ..\regnodes.h ..\regcharclass.h

..\regexec$(o) : ..\regnodes.h ..\regcharclass.h

reonly : regnodes .\config.h ..\git_version.h $(GLOBEXE) $(CONFIGPM) \
	$(UNIDATAFILES) $(PERLEXE) Extensions_reonly
	@echo	Perl and 're' are up to date.

static: $(PERLEXESTATIC)

#------------------------------------------------------------

$(GLOBEXE) : perlglob$(o)
	$(LINK32) $(BLINK_FLAGS) $(LIBFILES) -out:$@ \
	    perlglob$(o) setargv$(o)

perlglob$(o)  : perlglob.c

..\git_version.h : $(MINIPERL) ..\make_patchnum.pl
	cd .. && miniperl -Ilib make_patchnum.pl && cd win32

# make sure that we recompile perl.c if the git version changes
..\perl$(o) : ..\git_version.h

..\config.sh : $(CFGSH_TMPL) config_sh.PL FindExt.pm $(MINIPERL)
	$(MINIPERL) -I..\lib config_sh.PL $(CFG_VARS) $(CFGSH_TMPL) > ..\config.sh.tmp
	if exist ..\config.sh del /f ..\config.sh
	rename ..\config.sh.tmp config.sh

# This target is for when changes to the main config.sh happen.
# Edit config.vc, then make perl in a minimal configuration (i.e. with MULTI,
# ITHREADS, IMP_SYS and LARGE_FILES off), then make this target
# to regenerate config_H.vc.
regen_config_h:
	$(MINIPERL) -I..\lib config_sh.PL --prebuilt $(CFG_VARS) $(CFGSH_TMPL) > ..\config.sh
	$(MINIPERL) -I..\lib ..\configpm --chdir=..
	-del /f $(CFGH_TMPL)
	-$(MINIPERL) -I..\lib config_h.PL
	rename config.h $(CFGH_TMPL)

$(CONFIGPM) : $(MINIPERL) ..\config.sh config_h.PL ..\git_version.h
	$(MINIPERL) -I..\lib ..\configpm --chdir=..
	$(XCOPY) ..\*.h $(COREDIR)\*.*
	$(XCOPY) *.h $(COREDIR)\*.*
	$(RCOPY) include $(COREDIR)\*.*
	-$(MINIPERL) -I..\lib config_h.PL
	if errorlevel 1 $(MAKE) /$(MAKEFLAGS) $(CONFIGPM)

.\config.h : $(CONFIGPM)

# See the comment in Makefile.SH explaining this seemingly cranky ordering
$(MINIPERL) : ..\lib\buildcustomize.pl

..\lib\buildcustomize.pl : $(MINI_OBJ) ..\write_buildcustomize.pl
	$(LINK32) -out:$(MINIPERL) @<<
	$(BLINK_FLAGS) $(DELAYLOAD) $(MINIDELAYLOAD) $(LIBFILES) $(MINI_OBJ)
<<
	$(MINIPERL) -I..\lib -f ..\write_buildcustomize.pl ..

$(MINIDIR)\.exists : $(CFGH_TMPL)
	if not exist "$(MINIDIR)" mkdir "$(MINIDIR)"
#
# Copy the template config.h and set configurables at the end of it
# as per the options chosen and compiler used.
# Note: This config.h is only used to build miniperl.exe anyway, but
# it's as well to have its options correct to be sure that it builds
# and so that it's "-V" options are correct for use by makedef.pl. The
# real config.h used to build perl.exe is generated from the top-level
# config_h.SH by config_h.PL (run by miniperl.exe).
#
	copy $(CFGH_TMPL) config.h
	@(echo.&& \
	echo #ifndef _config_h_footer_&& \
	echo #define _config_h_footer_&& \
	echo #undef PTRSIZE&& \
	echo #undef SSize_t&& \
	echo #undef HAS_ATOLL&& \
	echo #undef HAS_STRTOLL&& \
	echo #undef HAS_STRTOULL&& \
	echo #undef Size_t_size&& \
	echo #undef IVTYPE&& \
	echo #undef UVTYPE&& \
	echo #undef IVSIZE&& \
	echo #undef UVSIZE&& \
	echo #undef NV_PRESERVES_UV&& \
	echo #undef NV_PRESERVES_UV_BITS&& \
	echo #undef IVdf&& \
	echo #undef UVuf&& \
	echo #undef UVof&& \
	echo #undef UVxf&& \
	echo #undef UVXf&& \
	echo #undef USE_64_BIT_INT&& \
	echo #undef USE_LONG_DOUBLE&& \
	echo #undef USE_CPLUSPLUS)>> config.h
!IF "$(CCTYPE)" != "MSVC120"
	@(echo #undef FILE_ptr&& \
	echo #undef FILE_cnt&& \
	echo #undef FILE_base&& \
	echo #undef FILE_bufsiz&& \
	echo #define FILE_ptr^(fp^) PERLIO_FILE_ptr^(fp^)&& \
	echo #define FILE_cnt^(fp^) PERLIO_FILE_cnt^(fp^)&& \
	echo #define FILE_base^(fp^) PERLIO_FILE_base^(fp^)&& \
	echo #define FILE_bufsiz^(fp^) ^(PERLIO_FILE_cnt^(fp^) + PERLIO_FILE_ptr^(fp^) - PERLIO_FILE_base^(fp^)^)&& \
	echo #define I_STDBOOL)>> config.h
!ENDIF
!IF "$(WIN64)"=="define"
	@(echo #define PTRSIZE ^8&& \
	echo #define SSize_t $(INT64)&& \
	echo #define HAS_ATOLL&& \
	echo #define HAS_STRTOLL&& \
	echo #define HAS_STRTOULL&& \
	echo #define Size_t_size ^8)>> config.h
!ELSE
	@(echo #define PTRSIZE ^4&& \
	echo #define SSize_t int&& \
	echo #undef HAS_ATOLL&& \
	echo #undef HAS_STRTOLL&& \
	echo #undef HAS_STRTOULL&& \
	echo #define Size_t_size ^4)>> config.h
!ENDIF
!IF "$(USE_64_BIT_INT)"=="define"
	@(echo #define IVTYPE $(INT64)&& \
	echo #define UVTYPE unsigned $(INT64)&& \
	echo #define IVSIZE ^8&& \
	echo #define UVSIZE ^8&& \
	echo #undef NV_PRESERVES_UV&& \
	echo #define NV_PRESERVES_UV_BITS 53&& \
	echo #define IVdf "I64d"&& \
	echo #define UVuf "I64u"&& \
	echo #define UVof "I64o"&& \
	echo #define UVxf "I64x"&& \
	echo #define UVXf "I64X"&& \
	echo #define USE_64_BIT_INT)>> config.h
!ELSE
	@(echo #define IVTYPE long&& \
	echo #define UVTYPE unsigned long&& \
	echo #define IVSIZE ^4&& \
	echo #define UVSIZE ^4&& \
	echo #define NV_PRESERVES_UV&& \
	echo #define NV_PRESERVES_UV_BITS 32&& \
	echo #define IVdf "ld"&& \
	echo #define UVuf "lu"&& \
	echo #define UVof "lo"&& \
	echo #define UVxf "lx"&& \
	echo #define UVXf "lX"&& \
	echo #undef USE_64_BIT_INT)>> config.h
!ENDIF
!IF "$(USE_CPLUSPLUS)"=="define"
	@(echo #define USE_CPLUSPLUS&& \
	echo #endif)>> config.h
!ELSE
	@(echo #undef USE_CPLUSPLUS&& \
	echo #endif)>> config.h
!ENDIF
#separate line since this is sentinal that this target is done
	@rem. > $(MINIDIR)\.exists

$(MINICORE_OBJ) : $(CORE_NOCFG_H)
	$(CC) -c $(CFLAGS) -DPERL_EXTERNAL_GLOB -DPERL_IS_MINIPERL $(OBJOUT_FLAG)$@ ..\$(*F).c

$(MINIWIN32_OBJ) : $(CORE_NOCFG_H)
	$(CC) -c $(CFLAGS) -DPERL_IS_MINIPERL $(OBJOUT_FLAG)$@ $(*F).c

# -DPERL_IMPLICIT_SYS needs C++ for perllib.c
# This is the only file that depends on perlhost.h, vmem.h, and vdir.h
!IF "$(USE_IMP_SYS)" == "define"
perllib$(o)	: perllib.c .\perlhost.h .\vdir.h .\vmem.h
	$(CC) -c -I. $(CFLAGS_O) $(CXX_FLAG) $(OBJOUT_FLAG)$@ perllib.c
!ENDIF

# 1. we don't want to rebuild miniperl.exe when config.h changes
# 2. we don't want to rebuild miniperl.exe with non-default config.h
# 3. we can't have miniperl.exe depend on git_version.h, as miniperl creates it
$(MINI_OBJ)	: $(MINIDIR)\.exists $(CORE_NOCFG_H)

$(WIN32_OBJ)	: $(CORE_H)
$(CORE_OBJ)	: $(CORE_H)
$(DLL_OBJ)	: $(CORE_H)

perldll.def : $(MINIPERL) $(CONFIGPM) ..\embed.fnc ..\makedef.pl create_perllibst_h.pl ..\git_version.h
	$(MINIPERL) -I..\lib create_perllibst_h.pl
	$(MINIPERL) -I..\lib -w ..\makedef.pl PLATFORM=win32 $(OPTIMIZE) $(DEFINES) $(BUILDOPT) \
	    CCTYPE=$(CCTYPE) TARG_DIR=..\ > perldll.def

$(PERLDLL): perldll.def $(PERLDLL_OBJ) $(PERLDLL_RES) Extensions_static
	$(LINK32) -dll -def:perldll.def -out:$@ @Extensions_static @<<
		$(BLINK_FLAGS) $(DELAYLOAD) $(LIBFILES) $(PERLDLL_OBJ) $(PERLDLL_RES)
<<
	$(XCOPY) $(PERLIMPLIB) $(COREDIR)

$(PERLSTATICLIB): Extensions_static
	$(LIB32) $(LIB_FLAGS) -out:$@ @Extensions_static @<<
		$(PERLDLL_OBJ)
<<
	$(XCOPY) $(PERLSTATICLIB) $(COREDIR)

$(PERLEXE_RES): perlexe.rc $(PERLEXE_MANIFEST) $(PERLEXE_ICO)

$(MINIDIR)\globals$(o) : $(GENERATED_HEADERS)

$(UUDMAP_H) $(MG_DATA_H) : $(BITCOUNT_H)

$(BITCOUNT_H) : $(GENUUDMAP)
	$(GENUUDMAP) $(GENERATED_HEADERS)

$(GENUUDMAP_OBJ) : ..\mg_raw.h

$(GENUUDMAP) : $(GENUUDMAP_OBJ)
	$(LINK32) -out:$@ @<<
		$(BLINK_FLAGS) $(LIBFILES) $(GENUUDMAP_OBJ)
<<

perlmain.c : runperl.c
	copy runperl.c perlmain.c

perlmain$(o) : perlmain.c
	$(CC) $(CFLAGS_O:-DPERLDLL=-UPERLDLL) $(OBJOUT_FLAG)$@ -c perlmain.c

perlmainst.c : runperl.c
	copy runperl.c perlmainst.c

perlmainst$(o) : perlmainst.c
	$(CC) $(CFLAGS_O) $(OBJOUT_FLAG)$@ -c perlmainst.c

$(PERLEXE): $(PERLDLL) $(CONFIGPM) $(PERLEXE_OBJ) $(PERLEXE_RES)
	$(LINK32) -out:$@ $(BLINK_FLAGS) \
	    $(PERLEXE_OBJ) $(PERLEXE_RES) $(PERLIMPLIB) $(LIBFILES) $(SETARGV_OBJ)
	copy $(PERLEXE) $(WPERLEXE)
	$(MINIPERL) -I..\lib bin\exetype.pl $(WPERLEXE) WINDOWS

$(PERLEXESTATIC): $(PERLSTATICLIB) $(CONFIGPM) $(PERLEXEST_OBJ) $(PERLEXE_RES)
	$(LINK32) -out:$@ $(BLINK_FLAGS) \
	    $(PERLEXEST_OBJ) $(PERLEXE_RES) $(PERLSTATICLIB) $(LIBFILES) $(SETARGV_OBJ)

MakePPPort: $(MINIPERL) $(CONFIGPM) Extensions_nonxs
	$(MINIPERL) -I..\lib ..\mkppport

#-------------------------------------------------------------------------------
# There's no direct way to mark a dependency on
# DynaLoader.pm, so this will have to do
Extensions: ..\make_ext.pl ..\lib\buildcustomize.pl $(PERLDEP) $(CONFIGPM) $(DYNALOADER)
	$(XCOPY) ..\*.h $(COREDIR)\*.*
	$(MINIPERL) -I..\lib ..\make_ext.pl "MAKE=$(MAKE)" --dir=$(CPANDIR) --dir=$(DISTDIR) --dir=$(EXTDIR) --dynamic

Extensions_reonly: ..\make_ext.pl ..\lib\buildcustomize.pl $(PERLDEP) $(CONFIGPM) $(DYNALOADER)
	$(XCOPY) ..\*.h $(COREDIR)\*.*
	$(MINIPERL) -I..\lib ..\make_ext.pl "MAKE=$(MAKE)" --dir=$(CPANDIR) --dir=$(DISTDIR) --dir=$(EXTDIR) --dynamic +re

Extensions_static : ..\make_ext.pl ..\lib\buildcustomize.pl list_static_libs.pl $(PERLDEP) $(CONFIGPM) Extensions_nonxs
	$(XCOPY) ..\*.h $(COREDIR)\*.*
	$(MINIPERL) -I..\lib ..\make_ext.pl "MAKE=$(MAKE)" --dir=$(CPANDIR) --dir=$(DISTDIR) --dir=$(EXTDIR) --static
	$(MINIPERL) -I..\lib list_static_libs.pl > Extensions_static

Extensions_nonxs: ..\make_ext.pl ..\lib\buildcustomize.pl $(PERLDEP) $(CONFIGPM) ..\pod\perlfunc.pod
	$(XCOPY) ..\*.h $(COREDIR)\*.*
	$(MINIPERL) -I..\lib ..\make_ext.pl "MAKE=$(MAKE)" --dir=$(CPANDIR) --dir=$(DISTDIR) --dir=$(EXTDIR) --nonxs

$(DYNALOADER) : ..\make_ext.pl ..\lib\buildcustomize.pl $(PERLDEP) $(CONFIGPM) Extensions_nonxs
	$(XCOPY) ..\*.h $(COREDIR)\*.*
	$(MINIPERL) -I..\lib ..\make_ext.pl "MAKE=$(MAKE)" --dir=$(EXTDIR) --dynaloader

Extensions_clean: 
	-if exist $(MINIPERL) $(MINIPERL) -I..\lib ..\make_ext.pl "MAKE=$(MAKE)" --dir=$(CPANDIR) --dir=$(DISTDIR) --dir=$(EXTDIR) --all --target=clean

Extensions_realclean: 
	-if exist $(MINIPERL) $(MINIPERL) -I..\lib ..\make_ext.pl "MAKE=$(MAKE)" --dir=$(CPANDIR) --dir=$(DISTDIR) --dir=$(EXTDIR) --all --target=realclean

#-------------------------------------------------------------------------------

doc: $(PERLEXE) ..\pod\perltoc.pod
	$(PERLEXE) ..\installhtml --podroot=.. --htmldir=$(HTMLDIR) \
	    --podpath=pod:lib:utils --htmlroot="file://$(INST_HTML::=|)" \
	    --recurse

..\utils\Makefile: $(CONFIGPM) ..\utils\Makefile.PL
	$(MINIPERL) -I..\lib ..\utils\Makefile.PL ..

# Note that this next section is parsed (and regenerated) by pod/buildtoc
# so please check that script before making structural changes here

utils: $(PERLEXE) ..\utils\Makefile
	cd ..\utils
	$(MAKE) PERL=$(MINIPERL)
	cd ..\pod
	copy ..\README.aix      ..\pod\perlaix.pod
	copy ..\README.amiga    ..\pod\perlamiga.pod
	copy ..\README.android  ..\pod\perlandroid.pod
	copy ..\README.bs2000   ..\pod\perlbs2000.pod
	copy ..\README.cn       ..\pod\perlcn.pod
	copy ..\README.cygwin   ..\pod\perlcygwin.pod
	copy ..\README.freebsd  ..\pod\perlfreebsd.pod
	copy ..\README.haiku    ..\pod\perlhaiku.pod
	copy ..\README.hpux     ..\pod\perlhpux.pod
	copy ..\README.hurd     ..\pod\perlhurd.pod
	copy ..\README.irix     ..\pod\perlirix.pod
	copy ..\README.jp       ..\pod\perljp.pod
	copy ..\README.ko       ..\pod\perlko.pod
	copy ..\README.linux    ..\pod\perllinux.pod
	copy ..\README.macosx   ..\pod\perlmacosx.pod
	copy ..\README.openbsd  ..\pod\perlopenbsd.pod
	copy ..\README.os2      ..\pod\perlos2.pod
	copy ..\README.os390    ..\pod\perlos390.pod
	copy ..\README.os400    ..\pod\perlos400.pod
	copy ..\README.plan9    ..\pod\perlplan9.pod
	copy ..\README.qnx      ..\pod\perlqnx.pod
	copy ..\README.riscos   ..\pod\perlriscos.pod
	copy ..\README.solaris  ..\pod\perlsolaris.pod
	copy ..\README.synology ..\pod\perlsynology.pod
	copy ..\README.tru64    ..\pod\perltru64.pod
	copy ..\README.tw       ..\pod\perltw.pod
	copy ..\README.vos      ..\pod\perlvos.pod
	copy ..\README.win32    ..\pod\perlwin32.pod
	copy ..\pod\perldelta.pod ..\pod\perl5382delta.pod
	cd ..\win32
	$(PERLEXE) $(PL2BAT) $(UTILS)
	$(MINIPERL) -I..\lib ..\autodoc.pl ..
	$(MINIPERL) -I..\lib ..\pod\perlmodlib.PL -q ..

..\pod\perltoc.pod: $(PERLEXE) Extensions Extensions_nonxs
	$(PERLEXE) -f ..\pod\buildtoc -q

# Note that the pod cleanup in this next section is parsed (and regenerated
# by pod/buildtoc so please check that script before making changes here

distclean: realclean
	-del /f $(MINIPERL) $(PERLEXE) $(PERLDLL) $(GLOBEXE) \
		$(PERLIMPLIB) ..\miniperl.lib $(PERLEXESTATIC) $(PERLSTATICLIB)
	-del /f $(LIBDIR)\Encode.pm $(LIBDIR)\encoding.pm $(LIBDIR)\Errno.pm
	-del /f $(LIBDIR)\Config.pod $(LIBDIR)\POSIX.pod $(LIBDIR)\threads.pm
	-del /f $(LIBDIR)\.exists $(LIBDIR)\attributes.pm $(LIBDIR)\DynaLoader.pm
	-del /f $(LIBDIR)\Fcntl.pm $(LIBDIR)\IO.pm $(LIBDIR)\Opcode.pm
	-del /f $(LIBDIR)\ops.pm $(LIBDIR)\Safe.pm
	-del /f $(LIBDIR)\SDBM_File.pm $(LIBDIR)\Socket.pm $(LIBDIR)\POSIX.pm
	-del /f $(LIBDIR)\B.pm $(LIBDIR)\O.pm $(LIBDIR)\re.pm
	-del /f $(LIBDIR)\File\Glob.pm
	-del /f $(LIBDIR)\Storable.pm
	-del /f $(LIBDIR)\Sys\Hostname.pm
	-del /f $(LIBDIR)\Time\HiRes.pm
	-del /f $(LIBDIR)\Unicode\Normalize.pm
	-del /f $(LIBDIR)\Math\BigInt\FastCalc.pm
	-del /f $(LIBDIR)\Win32.pm
	-del /f $(LIBDIR)\Win32CORE.pm
	-del /f $(LIBDIR)\Win32API\File.pm
	-del /f $(LIBDIR)\Win32API\File\cFile.pc
	-del /f $(LIBDIR)\buildcustomize.pl
	-del /f $(DISTDIR)\XSLoader\XSLoader.pm
	-del /f *.def *.map
	-if exist $(LIBDIR)\Amiga rmdir /s /q $(LIBDIR)\Amiga
	-if exist $(LIBDIR)\App rmdir /s /q $(LIBDIR)\App
	-if exist $(LIBDIR)\Archive rmdir /s /q $(LIBDIR)\Archive
	-if exist $(LIBDIR)\Attribute rmdir /s /q $(LIBDIR)\Attribute
	-if exist $(LIBDIR)\autodie rmdir /s /q $(LIBDIR)\autodie
	-if exist $(LIBDIR)\Carp rmdir /s /q $(LIBDIR)\Carp
	-if exist $(LIBDIR)\Compress rmdir /s /q $(LIBDIR)\Compress
	-if exist $(LIBDIR)\Config\Perl rmdir /s /q $(LIBDIR)\Config\Perl
	-if exist $(LIBDIR)\CPAN rmdir /s /q $(LIBDIR)\CPAN
	-if exist $(LIBDIR)\Data rmdir /s /q $(LIBDIR)\Data
	-if exist $(LIBDIR)\Devel rmdir /s /q $(LIBDIR)\Devel
	-if exist $(LIBDIR)\Digest rmdir /s /q $(LIBDIR)\Digest
	-if exist $(LIBDIR)\Encode rmdir /s /q $(LIBDIR)\Encode
	-if exist $(LIBDIR)\encoding rmdir /s /q $(LIBDIR)\encoding
	-if exist $(LIBDIR)\Exporter rmdir /s /q $(LIBDIR)\Exporter
	-if exist $(LIBDIR)\ExtUtils\CBuilder rmdir /s /q $(LIBDIR)\ExtUtils\CBuilder
	-if exist $(LIBDIR)\ExtUtils\Command rmdir /s /q $(LIBDIR)\ExtUtils\Command
	-if exist $(LIBDIR)\ExtUtils\Constant rmdir /s /q $(LIBDIR)\ExtUtils\Constant
	-if exist $(LIBDIR)\ExtUtils\Liblist rmdir /s /q $(LIBDIR)\ExtUtils\Liblist
	-if exist $(LIBDIR)\ExtUtils\MakeMaker rmdir /s /q $(LIBDIR)\ExtUtils\MakeMaker
	-if exist $(LIBDIR)\ExtUtils\ParseXS rmdir /s /q $(LIBDIR)\ExtUtils\ParseXS
	-if exist $(LIBDIR)\ExtUtils\Typemaps rmdir /s /q $(LIBDIR)\ExtUtils\Typemaps
	-if exist $(LIBDIR)\File\Spec rmdir /s /q $(LIBDIR)\File\Spec
	-if exist $(LIBDIR)\Filter rmdir /s /q $(LIBDIR)\Filter
	-if exist $(LIBDIR)\Hash rmdir /s /q $(LIBDIR)\Hash
	-if exist $(LIBDIR)\HTTP rmdir /s /q $(LIBDIR)\HTTP
	-if exist $(LIBDIR)\I18N rmdir /s /q $(LIBDIR)\I18N
	-if exist $(LIBDIR)\inc rmdir /s /q $(LIBDIR)\inc
	-if exist $(LIBDIR)\IO rmdir /s /q $(LIBDIR)\IO
	-if exist $(LIBDIR)\IPC rmdir /s /q $(LIBDIR)\IPC
	-if exist $(LIBDIR)\JSON rmdir /s /q $(LIBDIR)\JSON
	-if exist $(LIBDIR)\List rmdir /s /q $(LIBDIR)\List
	-if exist $(LIBDIR)\Locale rmdir /s /q $(LIBDIR)\Locale
	-if exist $(LIBDIR)\Math rmdir /s /q $(LIBDIR)\Math
	-if exist $(LIBDIR)\Memoize rmdir /s /q $(LIBDIR)\Memoize
	-if exist $(LIBDIR)\MIME rmdir /s /q $(LIBDIR)\MIME
	-if exist $(LIBDIR)\Module rmdir /s /q $(LIBDIR)\Module
	-if exist $(LIBDIR)\Net\FTP rmdir /s /q $(LIBDIR)\Net\FTP
	-if exist $(LIBDIR)\Params rmdir /s /q $(LIBDIR)\Params
	-if exist $(LIBDIR)\Parse rmdir /s /q $(LIBDIR)\Parse
	-if exist $(LIBDIR)\Perl rmdir /s /q $(LIBDIR)\Perl
	-if exist $(LIBDIR)\PerlIO rmdir /s /q $(LIBDIR)\PerlIO
	-if exist $(LIBDIR)\Pod\Html rmdir /s /q $(LIBDIR)\Pod\Html
	-if exist $(LIBDIR)\Pod\Perldoc rmdir /s /q $(LIBDIR)\Pod\Perldoc
	-if exist $(LIBDIR)\Pod\Simple rmdir /s /q $(LIBDIR)\Pod\Simple
	-if exist $(LIBDIR)\Pod\Text rmdir /s /q $(LIBDIR)\Pod\Text
	-if exist $(LIBDIR)\Scalar rmdir /s /q $(LIBDIR)\Scalar
	-if exist $(LIBDIR)\Search rmdir /s /q $(LIBDIR)\Search
	-if exist $(LIBDIR)\Sub rmdir /s /q $(LIBDIR)\Sub
	-if exist $(LIBDIR)\Sys rmdir /s /q $(LIBDIR)\Sys
	-if exist $(LIBDIR)\TAP rmdir /s /q $(LIBDIR)\TAP
	-if exist $(LIBDIR)\Term rmdir /s /q $(LIBDIR)\Term
	-if exist $(LIBDIR)\Test rmdir /s /q $(LIBDIR)\Test
	-if exist $(LIBDIR)\Test2 rmdir /s /q $(LIBDIR)\Test2
	-if exist $(LIBDIR)\Text rmdir /s /q $(LIBDIR)\Text
	-if exist $(LIBDIR)\Thread rmdir /s /q $(LIBDIR)\Thread
	-if exist $(LIBDIR)\threads rmdir /s /q $(LIBDIR)\threads
	-if exist $(LIBDIR)\Tie\Hash rmdir /s /q $(LIBDIR)\Tie\Hash
	-if exist $(LIBDIR)\Unicode\Collate rmdir /s /q $(LIBDIR)\Unicode\Collate
	-if exist $(LIBDIR)\Unicode\Collate\Locale rmdir /s /q $(LIBDIR)\Unicode\Collate\Locale
	-if exist $(LIBDIR)\version rmdir /s /q $(LIBDIR)\version
	-if exist $(LIBDIR)\VMS rmdir /s /q $(LIBDIR)\VMS
	-if exist $(LIBDIR)\Win32API rmdir /s /q $(LIBDIR)\Win32API
	-if exist $(LIBDIR)\XS rmdir /s /q $(LIBDIR)\XS
	-cd $(PODDIR) && del /f *.html *.bat roffitall \
	    perl5382delta.pod perlaix.pod perlamiga.pod perlandroid.pod \
	    perlapi.pod perlbs2000.pod perlcn.pod perlcygwin.pod \
	    perlfreebsd.pod perlhaiku.pod perlhpux.pod perlhurd.pod \
	    perlintern.pod perlirix.pod perljp.pod perlko.pod perllinux.pod \
	    perlmacosx.pod perlmodlib.pod perlopenbsd.pod perlos2.pod \
	    perlos390.pod perlos400.pod perlplan9.pod perlqnx.pod \
	    perlriscos.pod perlsolaris.pod perlsynology.pod perltoc.pod \
	    perltru64.pod perltw.pod perluniprops.pod perlvos.pod \
	    perlwin32.pod
	-cd ..\utils && del /f h2ph splain perlbug pl2pm h2xs \
	    perldoc perlivp libnetcfg enc2xs encguess piconv cpan streamzip *.bat \
	    xsubpp pod2html instmodsh json_pp prove ptar ptardiff ptargrep shasum corelist zipdetails
	-del /f ..\config.sh perlmain.c dlutils.c config.h.new \
	        perlmainst.c
	-del /f $(CONFIGPM)
	-del /f ..\lib\Config_git.pl
	-del /f bin\*.bat
	-del /f perllibst.h
	-del /f $(PERLEXE_RES) perl.base
	-cd .. && del /s *.lib *.map *.pdb *.ilk *.bs *$(o) .exists pm_to_blib ppport.h
	-cd $(EXTDIR) && del /s *.def Makefile Makefile.old
	-cd $(DISTDIR) && del /s *.def Makefile Makefile.old
	-cd $(CPANDIR) && del /s *.def Makefile Makefile.old
	-del /s ..\utils\Makefile
	-if exist $(AUTODIR) rmdir /s /q $(AUTODIR)
	-if exist $(COREDIR) rmdir /s /q $(COREDIR)
	-if exist pod2htmd.tmp del pod2htmd.tmp
	-if exist $(HTMLDIR) rmdir /s /q $(HTMLDIR)
	-del /f ..\t\test_state

install : all installbare installhtml

installbare : utils ..\pod\perltoc.pod
	$(PERLEXE) ..\installperl
	if exist $(WPERLEXE) $(XCOPY) $(WPERLEXE) $(INST_BIN)\*.*
	if exist $(PERLEXESTATIC) $(XCOPY) $(PERLEXESTATIC) $(INST_BIN)\*.*
	$(XCOPY) $(GLOBEXE) $(INST_BIN)\*.*
	if exist ..\perl*.pdb $(XCOPY) ..\perl*.pdb $(INST_BIN)\*.*
	$(XCOPY) bin\*.bat $(INST_SCRIPT)\*.*

installhtml : doc
	$(RCOPY) $(HTMLDIR)\*.* $(INST_HTML)\*.*

inst_lib : $(CONFIGPM)
	$(RCOPY) ..\lib $(INST_LIB)\*.*

$(UNIDATAFILES) ..\pod\perluniprops.pod : $(MINIPERL) $(CONFIGPM) ..\lib\unicore\mktables Extensions_nonxs
	cd ..\lib\unicore && \
	..\$(MINIPERL) -I.. mktables -P ..\..\pod -maketest -makelist -p -check $@ $(FIRSTUNIFILE)

minitest : .\config.h $(MINIPERL) ..\git_version.h $(GLOBEXE) $(CONFIGPM) $(UNIDATAFILES)
	$(XCOPY) $(MINIPERL) ..\t\$(NULL)
	if exist ..\t\perl.exe del /f ..\t\perl.exe
	rename ..\t\miniperl.exe perl.exe
	$(XCOPY) $(GLOBEXE) ..\t\$(NULL)
# Note this perl.exe is miniperl
	cd ..\t && perl.exe TEST base/*.t comp/*.t cmd/*.t run/*.t io/*.t re/*.t opbasic/*.t op/*.t uni/*.t perf/*.t pragma/*.t

test-prep : all utils ../pod/perltoc.pod
	$(XCOPY) $(PERLEXE) ..\t\$(NULL)
	$(XCOPY) $(PERLDLL) ..\t\$(NULL)
	$(XCOPY) $(GLOBEXE) ..\t\$(NULL)
	set PERL_STATIC_EXT=$(STATIC_EXT)

test : test-prep
	cd ..\t
	perl.exe harness $(TEST_ARGS) $(TEST_SWITCHES) $(TEST_FILES)
	cd ..\win32

test_porting : test-prep
	cd ..\t
	perl.exe harness $(TEST_ARGS) $(TEST_SWITCHES) porting\*.t ..\lib\diagnostics.t
	cd ..\win32

test-reonly : reonly utils
	$(XCOPY) $(PERLEXE) ..\t\$(NULL)
	$(XCOPY) $(PERLDLL) ..\t\$(NULL)
	$(XCOPY) $(GLOBEXE) ..\t\$(NULL)
	cd ..\t
	perl.exe harness $(OPT) -re \bre\\/ $(EXTRA)
	cd ..\win32

regen :
	cd ..
	regen.pl
	cd win32

test-notty : test-prep
	set PERL_SKIP_TTY_TEST=1
	cd ..\t
	perl.exe harness $(TEST_ARGS) $(TEST_SWITCHES) $(TEST_FILES)
	cd ..\win32

_test : 
       $(XCOPY) $(PERLEXE) ..\t\$(NULL)
       $(XCOPY) $(PERLDLL) ..\t\$(NULL)
       $(XCOPY) $(GLOBEXE) ..\t\$(NULL)
       cd ..\t
       perl.exe harness $(TEST_ARGS) $(TEST_SWITCHES) $(TEST_FILES)
       cd ..\win32

_clean :
	-@$(DEL) miniperlmain$(o)
	-@$(DEL) $(MINIPERL)
	-@$(DEL) perlglob$(o)
	-@$(DEL) perlmain$(o)
	-@$(DEL) perlmainst$(o)
	-@$(DEL) config.h
	-@$(DEL) ..\git_version.h
	-@$(DEL) $(GLOBEXE)
	-@$(DEL) $(PERLEXE)
	-@$(DEL) $(WPERLEXE)
	-@$(DEL) $(PERLEXESTATIC)
	-@$(DEL) $(PERLSTATICLIB)
	-@$(DEL) $(PERLDLL)
	-@$(DEL) $(CORE_OBJ)
	-@$(DEL) $(GENUUDMAP) $(GENUUDMAP_OBJ) $(GENERATED_HEADERS)
	-if exist $(MINIDIR) rmdir /s /q $(MINIDIR)
	-if exist $(UNIDATADIR1) rmdir /s /q $(UNIDATADIR1)
	-if exist $(UNIDATADIR2) rmdir /s /q $(UNIDATADIR2)
	-@$(DEL) $(UNIDATAFILES)
	-@$(DEL) $(WIN32_OBJ)
	-@$(DEL) $(DLL_OBJ)
	-@$(DEL) ..\*$(o) ..\*.lib ..\*.exp *$(o) *.lib *.exp *.res
	-@$(DEL) ..\t\*.exe ..\t\*.dll ..\t\*.bat
	-@$(DEL) *.ilk
	-@$(DEL) *.pdb
	-@$(DEL) Extensions_static

clean : Extensions_clean _clean

realclean : Extensions_realclean _clean

# Handy way to run perlbug -ok without having to install and run the
# installed perlbug. We don't re-run the tests here - we trust the user.
# Please *don't* use this unless all tests pass.
# If you want to report test failures, use "nmake nok" instead.
ok: utils
	$(PERLEXE) ..\utils\perlbug -ok -s "(UNINSTALLED)"

okfile: utils
	$(PERLEXE) ..\utils\perlbug -ok -s "(UNINSTALLED)" -F perl.ok

nok: utils
	$(PERLEXE) ..\utils\perlbug -nok -s "(UNINSTALLED)"

nokfile: utils
	$(PERLEXE) ..\utils\perlbug -nok -s "(UNINSTALLED)" -F perl.nok
