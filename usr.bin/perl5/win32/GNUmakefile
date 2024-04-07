#
# Makefile to build perl on Windows using GMAKE.
# Supported compilers:
#	Microsoft Visual C++ 12.0 or later
#	MinGW with gcc-3.4.5-5.3.0
#	MinGW64 with gcc-4.4.3 or later
#
# This is set up to build a perl.exe that runs off a shared library
# (perl538.dll).  Also makes individual DLLs for the XS extensions.
#
# The easiest way to customize the build process is to use parameters like this:
#
# c:\dev\perlsrc\win32> gmake INST_TOP=c:\sw\perl CCHOME=c:\sw\mingw USE_64_BIT_INT=define


##
## Make sure you read README.win32 *before* you mess with anything here!
##

#
# We set this to point to cmd.exe in case GNU Make finds sh.exe in the path.
# Comment this line out if necessary
#
SHELL := cmd.exe

# define whether you want to use native gcc compiler or cross-compiler
# possible values: gcc
#                  i686-w64-mingw32-gcc
#                  x86_64-w64-mingw32-gcc
GCCBIN := gcc

ifeq ($(GCCBIN),x86_64-w64-mingw32-gcc)
GCCCROSS := x86_64-w64-mingw32
endif
ifeq ($(GCCBIN),i686-w64-mingw32-gcc)
GCCCROSS := i686-w64-mingw32
endif


##
## Build configuration.  Edit the values below to suit your needs.
##

#
# Set these to wherever you want "gmake install" to put your
# newly built perl.
#
INST_DRV := c:
INST_TOP := $(INST_DRV)\perl

#
# Uncomment if you want to build a 32-bit Perl using a 32-bit compiler
# on a 64-bit version of Windows.
#
#WIN64		:= undef

#
# Comment this out if you DON'T want your perl installation to be versioned.
# This means that the new installation will overwrite any files from the
# old installation at the same INST_TOP location.  Leaving it enabled is
# the safest route, as perl adds the extra version directory to all the
# locations it installs files to.  If you disable it, an alternative
# versioned installation can be obtained by setting INST_TOP above to a
# path that includes an arbitrary version string.
#
#INST_VER	:= \5.38.2

#
# Comment this out if you DON'T want your perl installation to have
# architecture specific components.  This means that architecture-
# specific files will be installed along with the architecture-neutral
# files.  Leaving it enabled is safer and more flexible, in case you
# want to build multiple flavors of perl and install them together in
# the same location.  Commenting it out gives you a simpler
# installation that is easier to understand for beginners.
#
#INST_ARCH	:= \$(ARCHNAME)

#
# Uncomment this if you want perl to run
# 	$Config{sitelibexp}\sitecustomize.pl
# before anything else.  This script can then be set up, for example,
# to add additional entries to @INC.
#
#USE_SITECUST	:= define

#
# uncomment to enable multiple interpreters.  This is needed for fork()
# emulation and for thread support, and is auto-enabled by USE_IMP_SYS
# and USE_ITHREADS below.
#
USE_MULTI	:= define

#
# Interpreter cloning/threads; now reasonably complete.
# This should be enabled to get the fork() emulation.  This needs (and
# will auto-enable) USE_MULTI above.
#
USE_ITHREADS	:= define

#
# uncomment to enable the implicit "host" layer for all system calls
# made by perl.  This is also needed to get fork().  This needs (and
# will auto-enable) USE_MULTI above.
#
USE_IMP_SYS	:= define

#
# Comment out next assign to disable perl's I/O subsystem and use compiler's
# stdio for IO - depending on your compiler vendor and run time library you may
# then get a number of fails from make test i.e. bugs - complain to them not us ;-).
# You will also be unable to take full advantage of perl5.8's support for multiple
# encodings and may see lower IO performance. You have been warned.
#
USE_PERLIO	:= define

#
# Uncomment this if you're building a 32-bit perl and want 64-bit integers.
# (If you're building a 64-bit perl then you will have 64-bit integers whether
# or not this is uncommented.)
#
#USE_64_BIT_INT	:= define

#
# Uncomment this if you want to support the use of long doubles in GCC builds.
# This option is not supported for MSVC builds.
#
#USE_LONG_DOUBLE := define

#
# Uncomment these if you want to support the use of __float128 in GCC builds.
# This option is not supported for MSVC builds.
#
#USE_QUADMATH := define
#I_QUADMATH := define

#
# Comment this out if you want to build perl without __USE_MINGW_ANSI_STDIO defined.
# (If you're building perl with USE_LONG_DOUBLE defined then
# __USE_MINGW_ANSI_STDIO will be defined whether or not this is uncommented.)
# The advantage of defining __USE_MINGW_ANSI_STDIO is that it provides correct
# (s)printf formatting of numbers, whereas the MS runtime might not.
# This option has no effect on MSVC builds.
#
USE_MINGW_ANSI_STDIO := define

#
# Comment this out if you want the legacy default behavior of including '.' at
# the end of @INC.
#
DEFAULT_INC_EXCLUDES_DOT := define

#
# Uncomment this if you want to disable looking up values from
# HKEY_CURRENT_USER\Software\Perl and HKEY_LOCAL_MACHINE\Software\Perl in
# the Registry.
#
#USE_NO_REGISTRY := define

#
# uncomment exactly one of the following
#
# Visual C++ 2013 (aka Visual C++ 12.0) (full version or Express Edition)
#CCTYPE		:= MSVC120
# Visual C++ 2015 (aka Visual C++ 14.0) (full version or Express Edition)
#CCTYPE		:= MSVC140
# Visual C++ 2017 (aka Visual C++ 14.1) (full version or Community Edition)
#CCTYPE		:= MSVC141
# Visual C++ 2019 (aka Visual C++ 14.2) (full version or Community Edition)
#CCTYPE		:= MSVC142
# Visual C++ 2022 (aka Visual C++ 14.3) (full version or Community Edition)
#CCTYPE		:= MSVC143
# MinGW or mingw-w64 with gcc-3.4.5 or later
#CCTYPE		:= GCC

#
# If you are using Intel C++ Compiler uncomment this
#
#__ICC		:= define

#
# Uncomment this if you want to build everything in C++ mode
# This requires gcc or at least MSVC 2019.
#
#USE_CPLUSPLUS	:= define

#
# uncomment next line if you want debug version of perl (big/slow)
# If not enabled, we automatically try to use maximum optimization
# with all compilers that are known to have a working optimizer.
#
#CFG		:= Debug

#
# uncomment to enable linking with setargv.obj under the Visual C
# compiler. Setting this options enables perl to expand wildcards in
# arguments, but it may be harder to use alternate methods like
# File::DosGlob that are more powerful.  This option is supported only with
# Visual C.
#
#USE_SETARGV	:= define

#
# set this if you wish to use perl's malloc
# WARNING: Turning this on/off WILL break binary compatibility with extensions
# you may have compiled with/without it.  Be prepared to recompile all
# extensions if you change the default.  Currently, this cannot be enabled
# if you ask for USE_IMP_SYS above.
#
#PERL_MALLOC	:= define

#
# set this to enable debugging mstats
# This must be enabled to use the Devel::Peek::mstat() function.  This cannot
# be enabled without PERL_MALLOC as well.
#
#DEBUG_MSTATS	:= define

#
# set this to additionally provide a statically linked perl-static.exe.
# Note that dynamic loading will not work with this perl, so you must
# include required modules statically using the STATIC_EXT or ALL_STATIC
# variables below. A static library perl538s.lib will also be created.
# Ordinary perl.exe is not affected by this option.
#
#BUILD_STATIC	:= define

#
# in addition to BUILD_STATIC the option ALL_STATIC makes *every*
# extension get statically built.
# This will result in a very large perl executable, but the main purpose
# is to have proper linking set so as to be able to create miscellaneous
# executables with different built-in extensions. It implies BUILD_STATIC.
#
#ALL_STATIC	:= define

#
# set the install locations of the compiler
# Running VCVARS32.BAT, VCVARSALL.BAT or similar is *required* when using
# Visual C++.
#
# For GCC builds this should be the directory containing the bin, include,
# lib directories for your compiler.
#
#CCHOME		:= C:\MinGW

#
# Additional compiler flags can be specified here.
#
BUILDOPT	:= $(BUILDOPTEXTRA)

#
# This should normally be disabled.  Enabling it will disable the File::Glob
# implementation of CORE::glob.
#
#BUILDOPT	+= -DPERL_EXTERNAL_GLOB

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
BUILDOPT	+= -DPERL_TEXTMODE_SCRIPTS

#
# specify semicolon-separated list of extra directories that modules will
# look for libraries (spaces in path names need not be quoted)
#
EXTRALIBDIRS	:=

#
# set this to your email address (perl will guess a value from
# your loginname and your hostname, which may not be right)
#
#EMAIL		:=

##
## Build configuration ends.
##

##################### CHANGE THESE ONLY IF YOU MUST #####################

PERL_MALLOC	?= undef
DEBUG_MSTATS	?= undef

USE_SITECUST	?= undef
USE_MULTI	?= undef
USE_ITHREADS	?= undef
USE_IMP_SYS	?= undef
USE_PERLIO	?= undef
USE_LARGE_FILES	?= undef
USE_64_BIT_INT	?= undef
USE_LONG_DOUBLE	?= undef
USE_QUADMATH	?= undef
I_QUADMATH	?= undef
DEFAULT_INC_EXCLUDES_DOT ?= undef
USE_NO_REGISTRY	?= undef

ifeq ($(USE_IMP_SYS),define)
PERL_MALLOC	= undef
endif

ifeq ($(PERL_MALLOC),undef)
DEBUG_MSTATS	= undef
endif

ifeq ($(DEBUG_MSTATS),define)
BUILDOPT	+= -DPERL_DEBUGGING_MSTATS
endif

ifeq ("$(USE_IMP_SYS) $(USE_MULTI)","define undef")
USE_MULTI	= define
endif

ifeq ("$(USE_ITHREADS) $(USE_MULTI)","define undef")
USE_MULTI	= define
endif

ifeq ($(USE_SITECUST),define)
BUILDOPT	+= -DUSE_SITECUSTOMIZE
endif

ifneq ($(USE_MULTI),undef)
BUILDOPT	+= -DMULTIPLICITY
endif

ifneq ($(USE_IMP_SYS),undef)
BUILDOPT	+= -DPERL_IMPLICIT_SYS
endif

ifeq ($(USE_NO_REGISTRY),define)
BUILDOPT	+= -DWIN32_NO_REGISTRY
endif

ifeq ($(CCTYPE),GCC)
GCCTARGET	:= $(shell $(GCCBIN) -dumpmachine)
endif

#no explicit CCTYPE given, do auto detection
ifeq ($(CCTYPE),)
GCCTARGET	:= $(shell $(GCCBIN) -dumpmachine 2>NUL)
#do we have a GCC?
ifneq ($(GCCTARGET),)
CCTYPE		:= GCC
else
WIN64		:= $(shell for /f "tokens=3 delims=.^ " \
	%%i in ('cl ^2^>^&1') do @if "%%i" == "32-bit" echo undef)
#major version of CL has diff position based on 32 vs 64
#Microsoft (R) C/C++ Optimizing Compiler Version 15.00.30729.01 for x64
#Microsoft (R) 32-bit C/C++ Optimizing Compiler Version 15.00.30729.01 for 80x86
#use var to capture 1st line only, not 8th token of lines 2 & 3 in cl.exe output
#removing the cmd /c (extra scope) on the echo causes the env var to be undef
#when echo executes but it isn't undef for the "set MSVCVER", gmake or
#cmd.exe bug?
ifeq ($(WIN64),undef)
MSVCVER		:= $(shell (set MSVCVER=) & (for /f "tokens=8,9 delims=.^ " \
	%%i in ('cl ^2^>^&1') do if not defined MSVCVER if %%i%% geq 19 \
	(set /A "MSVCVER=((%%i-5)*10)+(%%j/10)") \
	else (set /A "MSVCVER=(%%i-6)*10")) & cmd /c echo %%MSVCVER%%)
else
MSVCVER		:= $(shell (set MSVCVER=) & (for /f "tokens=7,8 delims=.^ " \
	%%i in ('cl ^2^>^&1') do if not defined MSVCVER if %%i%% geq 19 \
	(set /A "MSVCVER=((%%i-5)*10)+(%%j/10)") \
	else (set /A "MSVCVER=(%%i-6)*10")) & cmd /c echo %%MSVCVER%%)
endif
#autodetect failed, reset to empty string
ifeq ($(MSVCVER),-50)
CCTYPE		:= 
else
CCTYPE		:= MSVC$(MSVCVER)
endif
endif
endif

# Versions of Visual C++ up to VC++ 14.0 define $(VCINSTALLDIR), but for
# VC++ 14.1 we need the subfolder given by $(VCToolsInstallDir).
ifeq ($(CCHOME),)
ifeq ($(CCTYPE),GCC)
CCHOME		:= C:\MinGW
else ifeq ($(CCTYPE),MSVC120)
CCHOME		:= $(VCINSTALLDIR)
else ifeq ($(CCTYPE),MSVC140)
CCHOME		:= $(VCINSTALLDIR)
else
CCHOME		:= $(VCToolsInstallDir)
endif
endif

# Check processor architecture from target triplet
# possible values:
# gcc: i686-w64-mingw32, x86_64-w64-mingw32
# clang: i686-w64-windows-gnu, x86_64-w64-windows-gnu
ifeq ($(CCTYPE),GCC)
ifeq (x86_64, $(findstring x86_64, $(GCCTARGET)))
WIN64 := define
PROCESSOR_ARCHITECTURE := x64
endif
ifeq (i686, $(findstring i686, $(GCCTARGET)))
WIN64 := undef
PROCESSOR_ARCHITECTURE := x86
endif
endif

PROCESSOR_ARCHITECTURE  ?= x86

ifeq ($(WIN64),undef)
PROCESSOR_ARCHITECTURE	= x86
endif

ifeq ($(WIN64),)
# When we are running from a 32bit cmd.exe on AMD64 then
# PROCESSOR_ARCHITECTURE is set to x86 and PROCESSOR_ARCHITEW6432
# is set to AMD64
ifneq ($(PROCESSOR_ARCHITEW6432),)
PROCESSOR_ARCHITECTURE	= $(PROCESSOR_ARCHITEW6432)
WIN64			= define
else ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
WIN64			= define
else ifeq ($(PROCESSOR_ARCHITECTURE),IA64)
WIN64			= define
else
WIN64			= undef
endif
endif

ifeq ($(WIN64),define)
USE_64_BIT_INT	= define
endif

# Disable the long double option for MSVC builds since that compiler
# does not support it.
ifneq ($(CCTYPE),GCC)
USE_LONG_DOUBLE	= undef
USE_QUADMATH = undef
I_QUADMATH = undef
endif

ARCHITECTURE = $(PROCESSOR_ARCHITECTURE)
ifeq ($(ARCHITECTURE),AMD64)
ARCHITECTURE	= x64
endif
ifeq ($(ARCHITECTURE),IA64)
ARCHITECTURE	= ia64
endif

ifeq ($(USE_MULTI),define)
ARCHNAME	= MSWin32-$(ARCHITECTURE)-multi
else
ifeq ($(USE_PERLIO),define)
ARCHNAME	= MSWin32-$(ARCHITECTURE)-perlio
else
ARCHNAME	= MSWin32-$(ARCHITECTURE)
endif
endif

ifeq ($(USE_PERLIO),define)
BUILDOPT	+= -DUSE_PERLIO
endif

ifeq ($(USE_ITHREADS),define)
ARCHNAME	:= $(ARCHNAME)-thread
endif

ifneq ($(WIN64),define)
ifeq ($(USE_64_BIT_INT),define)
ARCHNAME	:= $(ARCHNAME)-64int
endif
endif

ifeq ($(USE_LONG_DOUBLE),define)
ARCHNAME	:= $(ARCHNAME)-ld
endif

ifeq ($(USE_QUADMATH),define)
ARCHNAME	:= $(ARCHNAME)-quadmath
endif

ifeq ($(CCTYPE),GCC)
GCCVER		:= $(shell $(GCCBIN) -dumpversion)
GCCVER1		:= $(shell for /f "delims=. tokens=1,2,3" %%i in ('$(GCCBIN) -dumpversion') do echo %%i)
GCCVER2		:= $(shell for /f "delims=. tokens=1,2,3" %%i in ('$(GCCBIN) -dumpversion') do echo %%j)
GCCVER3		:= $(shell for /f "delims=. tokens=1,2,3" %%i in ('$(GCCBIN) -dumpversion') do echo %%k)
endif

# Set the install location of the compiler headers/libraries.
# These are saved into $Config{incpath} and $Config{libpth}.
ifneq ($(GCCCROSS),)
CCINCDIR := $(CCHOME)\$(GCCCROSS)\include
CCLIBDIR := $(CCHOME)\$(GCCCROSS)\lib
ARCHPREFIX := $(GCCCROSS)-
else ifeq ($(CCTYPE),GCC)
CCINCDIR := $(CCHOME)\include
CCLIBDIR := $(CCHOME)\lib;$(CCHOME)\$(GCCTARGET)\lib;$(CCHOME)\lib\gcc\$(GCCTARGET)\$(GCCVER)
ARCHPREFIX :=
else
CCINCDIR := $(CCHOME)\include
ifeq ($(CCTYPE),MSVC120)
ifeq ($(WIN64),define)
CCLIBDIR := $(CCHOME)\lib\amd64
else
CCLIBDIR := $(CCHOME)\lib
endif
else ifeq ($(CCTYPE),MSVC140)
ifeq ($(WIN64),define)
CCLIBDIR := $(CCHOME)\lib\amd64
else
CCLIBDIR := $(CCHOME)\lib
endif
else
ifeq ($(WIN64),define)
CCLIBDIR := $(CCHOME)\lib\x64
else
CCLIBDIR := $(CCHOME)\lib\x86
endif
endif
ARCHPREFIX :=
endif

# Set DLL location for GCC compilers.
ifeq ($(CCTYPE),GCC)
ifneq ($(GCCCROSS),)
CCDLLDIR := $(CCHOME)\$(GCCCROSS)\lib
else
CCDLLDIR := $(CCHOME)\bin
endif
endif

ARCHDIR		= ..\lib\$(ARCHNAME)
COREDIR		= ..\lib\CORE
AUTODIR		= ..\lib\auto
LIBDIR		= ..\lib
EXTDIR		= ..\ext
DISTDIR		= ..\dist
CPANDIR		= ..\cpan
PODDIR		= ..\pod
HTMLDIR		= .\html

# Strip trailing backslash from INST_TOP
override INST_TOP := $(INST_TOP:\=)

INST_SCRIPT	= $(INST_TOP)$(INST_VER)\bin
INST_BIN	= $(INST_SCRIPT)$(INST_ARCH)
INST_LIB	= $(INST_TOP)$(INST_VER)\lib
INST_ARCHLIB	= $(INST_LIB)$(INST_ARCH)
INST_COREDIR	= $(INST_ARCHLIB)\CORE
INST_HTML	= $(INST_TOP)$(INST_VER)\html

#
# Programs to compile, build .lib files and link
#

MINIBUILDOPT    :=

ifeq ($(CCTYPE),GCC)

CC		= $(ARCHPREFIX)gcc
LINK32		= $(ARCHPREFIX)g++
LIB32		= $(ARCHPREFIX)ar rc
IMPLIB		= $(ARCHPREFIX)dlltool
RSC		= $(ARCHPREFIX)windres

ifeq ($(USE_LONG_DOUBLE),define)
BUILDOPT        += -D__USE_MINGW_ANSI_STDIO
MINIBUILDOPT    += -D__USE_MINGW_ANSI_STDIO
else ifeq ($(USE_MINGW_ANSI_STDIO),define)
BUILDOPT        += -D__USE_MINGW_ANSI_STDIO
MINIBUILDOPT    += -D__USE_MINGW_ANSI_STDIO
endif


# By default add -fwrapv, since we depend on 2's complement behaviour
# for signed numbers.
# See https://github.com/Perl/perl5/issues/13690
#
GCCWRAPV := define

ifeq ($(GCCWRAPV),define)
BUILDOPT        += -fwrapv
MINIBUILDOPT    += -fwrapv
endif

i = .i
o = .o
a = .a

#
# Options
#

INCLUDES	= -I.\include -I. -I..
DEFINES		= -DWIN32
ifeq ($(WIN64),define)
DEFINES		+= -DWIN64
endif
LOCDEFS		= -DPERLDLL -DPERL_CORE
CXX_FLAG	= -xc++
LIBC		=
LIBFILES	= $(LIBC) -lmoldname -lkernel32 -luser32 -lgdi32 -lwinspool \
	-lcomdlg32 -ladvapi32 -lshell32 -lole32 -loleaut32 -lnetapi32 \
	-luuid -lws2_32 -lmpr -lwinmm -lversion -lodbc32 -lodbccp32 -lcomctl32

ifeq ($(USE_QUADMATH),define)
LIBFILES	+= -lquadmath
endif

ifeq ($(CFG),Debug)
# According to https://gcc.gnu.org/onlinedocs/gcc-8.3.0/gcc/Optimize-Options.html
# -Og should provide some optimizations while still giving convenient debugging
OPTIMIZE	= -g -Og
LINK_DBG	= -g
DEFINES		+= -DDEBUGGING
else
# In https://github.com/Perl/perl5/issues/20081 it is found that the previous
# optimization level -O2 causes generated code that fails in mysterious ways
# when run on Win11 (*even* if it was built and successfully tested on Win10!).
OPTIMIZE	= -Os
LINK_DBG	= -s
endif

EXTRACFLAGS	=
ifeq ($(USE_CPLUSPLUS),define)
EXTRACFLAGS	+= $(CXX_FLAG)
endif
CFLAGS		= $(EXTRACFLAGS) $(INCLUDES) $(DEFINES) $(LOCDEFS) $(OPTIMIZE)
LINK_FLAGS	= $(LINK_DBG) -L"$(INST_COREDIR)" -L"$(subst ;," -L",$(CCLIBDIR))"
OBJOUT_FLAG	= -o
EXEOUT_FLAG	= -o
LIBOUT_FLAG	=
PDBOUT		=

BUILDOPT	+= -fno-strict-aliasing -mms-bitfields
MINIBUILDOPT	+= -fno-strict-aliasing

TESTPREPGCC	= test-prep-gcc

else

o = .obj

# Loading DLLs on demand makes the test suite run in about 10% less time.
# If no registry, advapi32 is only used for Perl_pp_getlogin/getlogin/GetUserNameA
# which is rare to execute
ifneq ($(USE_NO_REGISTRY),undef)
DELAYLOAD	= -DELAYLOAD:ws2_32.dll -DELAYLOAD:advapi32.dll delayimp.lib
MINIDELAYLOAD	=
else
DELAYLOAD	= -DELAYLOAD:ws2_32.dll delayimp.lib
#miniperl never does any registry lookups
MINIDELAYLOAD	= -DELAYLOAD:advapi32.dll
endif

ifneq ($(__ICC),define)
CC		= cl
LINK32		= link
else
CC		= icl
LINK32		= xilink
endif
LIB32		= $(LINK32) -lib
RSC		= rc

#
# Options
#

INCLUDES	= -I.\include -I. -I..
#PCHFLAGS	= -Fpc:\temp\vcmoduls.pch -YX
DEFINES		= -DWIN32 -D_CONSOLE -DNO_STRICT
LOCDEFS		= -DPERLDLL -DPERL_CORE
CXX_FLAG	= -TP -EHsc
ifeq ($(USE_CPLUSPLUS),define)
CXX_FLAG	+= -std:c++20
endif
EXTRACFLAGS	= -nologo -GF -W3

ifeq ($(CCTYPE),MSVC120)
LIBC		= msvcrt.lib
else
LIBC		= ucrt.lib
endif

ifeq ($(CFG),Debug)
OPTIMIZE	= -Od -Zi
LINK_DBG	= -debug
DEFINES		+= -DDEBUGGING
EXTRACFLAGS	+= -MD
else ifeq ($(CFG),DebugSymbols)
OPTIMIZE	= -Od -Zi
LINK_DBG	= -debug
EXTRACFLAGS	+= -MD
else ifeq ($(CFG),DebugFull)
ifeq ($(CCTYPE),MSVC120)
LIBC		= msvcrtd.lib
else
LIBC		= ucrtd.lib
endif
OPTIMIZE	= -Od -Zi
LINK_DBG	= -debug
DEFINES		+= -D_DEBUG -DDEBUGGING
EXTRACFLAGS	+= -MDd

else
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
#LINK_DBG	+= -debugtype:both
LIB_FLAGS	= -ltcg
EXTRACFLAGS	+= -MD
endif

ifeq ($(WIN64),define)
DEFINES		+= -DWIN64
OPTIMIZE	+= -fp:precise
endif

# For now, silence warnings about "unsafe" CRT functions
# and POSIX CRT function names being deprecated.
DEFINES		+= -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE

# Likewise for deprecated Winsock APIs in VC++ 14.0 onwards for now.
ifneq ($(CCTYPE),MSVC120)
DEFINES		+= -D_WINSOCK_DEPRECATED_NO_WARNINGS
endif

LIBBASEFILES	= oldnames.lib kernel32.lib user32.lib gdi32.lib winspool.lib \
	comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib \
	netapi32.lib uuid.lib ws2_32.lib mpr.lib winmm.lib version.lib \
	odbc32.lib odbccp32.lib comctl32.lib

ifneq ($(CCTYPE),MSVC120)
ifeq ($(CFG),DebugFull)
LIBBASEFILES	+= msvcrtd.lib vcruntimed.lib
else
LIBBASEFILES	+= msvcrt.lib vcruntime.lib
endif
endif

# Avoid __intel_new_proc_init link error for libircmt.
# libmmd is /MD equivalent, other variants exist.
# libmmd is Intel C's math addon funcs to MS CRT, contains long doubles, C99,
# and optimized C89 funcs
ifeq ($(__ICC),define)
LIBBASEFILES	+= libircmt.lib libmmd.lib
endif

LIBFILES	= $(LIBBASEFILES) $(LIBC)

ifeq ($(__ICC),define)
EXTRACFLAGS	+= -Qstd=c99
endif
ifeq ($(USE_CPLUSPLUS),define)
EXTRACFLAGS	+= $(CXX_FLAG)
endif
CFLAGS		= $(EXTRACFLAGS) $(INCLUDES) $(DEFINES) $(LOCDEFS) \
		$(PCHFLAGS) $(OPTIMIZE)
LINK_FLAGS	= -nologo -nodefaultlib $(LINK_DBG) \
		-libpath:"$(INST_COREDIR)" \
		-machine:$(PROCESSOR_ARCHITECTURE)
LIB_FLAGS	+= -nologo
OBJOUT_FLAG	= -Fo
EXEOUT_FLAG	= -Fe
LIBOUT_FLAG	= /out:
PDBOUT		= -Fd$(*).pdb
TESTPREPGCC	=

endif

CFLAGS_O	= $(CFLAGS) $(BUILDOPT)

RSC_FLAGS	=

# VS 2017 (VC++ 14.1) requires at minimum Windows 7 SP1 (with latest Windows Updates)

# For XP support in >= VS 2013 (VC++ 12.0), subsystem is always in Config.pm
# LINK_FLAGS.

ifneq ($(CCTYPE),GCC)
ifeq ($(WIN64),define)
LINK_FLAGS	+= -subsystem:console,"5.02"
else
LINK_FLAGS	+= -subsystem:console,"5.01"
endif
endif

BLINK_FLAGS	= $(PRIV_LINK_FLAGS) $(LINK_FLAGS)

#################### do not edit below this line #######################
############# NO USER-SERVICEABLE PARTS BEYOND THIS POINT ##############

#prevent -j from reaching EUMM/make_ext.pl/"sub makes", Win32 EUMM not parallel
#compatible yet
unexport MAKEFLAGS

a ?= .lib

.SUFFIXES : .c .i $(o) .dll $(a) .exe .rc .res

%$(o): %.c
	$(CC) -c -I$(<D) $(CFLAGS_O) $(OBJOUT_FLAG)$@ $(PDBOUT) $<

%.i: %.c
	$(CC) -c -I$(<D) $(CFLAGS_O) -E $< >$@

%.c: %.y
	$(NOOP)

%.dll: %$(o)
	$(LINK32) -o $@ $(BLINK_FLAGS) $< $(LIBFILES)
	$(IMPLIB) --input-def $(*F).def --output-lib $(*F).a $@

%.res: %.rc
ifeq ($(CCTYPE),GCC)
	$(RSC) --use-temp-file --include-dir=. --include-dir=.. -O COFF -D INCLUDE_MANIFEST -i $< -o $@
else
	$(RSC) -i.. -DINCLUDE_MANIFEST $<
endif

#
# various targets

#do not put $(MINIPERL) as a dep/prereq in a rule, instead put $(HAVEMINIPERL)
#$(MINIPERL) is not a buildable target, use "gmake mp" if you want to just build
#miniperl alone
MINIPERL	= ..\miniperl.exe
HAVEMINIPERL	= ..\lib\buildcustomize.pl
MINIDIR		= mini
PERLEXE		= ..\perl.exe
WPERLEXE	= ..\wperl.exe
PERLEXESTATIC	= ..\perl-static.exe
STATICDIR	= .\static.tmp
GLOBEXE		= ..\perlglob.exe
CONFIGPM	= ..\lib\Config.pm
GENUUDMAP	= ..\generate_uudmap.exe
ifeq ($(BUILD_STATIC),define)
PERLSTATIC	= static
else
ifeq ($(ALL_STATIC),define)
PERLSTATIC	= static
else
PERLSTATIC	=
endif
endif

# Unicode data files generated by mktables
UNIDATAFILES	 = ..\lib\unicore\Decomposition.pl ..\lib\unicore\TestProp.pl \
		   ..\lib\unicore\CombiningClass.pl ..\lib\unicore\Name.pl \
		   ..\lib\unicore\UCD.pl ..\lib\unicore\Name.pm            \
		   ..\lib\unicore\mktables.lst

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
PERLDEP = $(PERLIMPLIB)


PL2BAT		= bin\pl2bat.pl

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
		..\utils\encguess	\
		..\utils\piconv		\
		..\utils\corelist	\
		..\utils\cpan		\
		..\utils\xsubpp		\
		..\utils\pod2html	\
		..\utils\prove		\
		..\utils\ptar		\
		..\utils\ptardiff	\
		..\utils\ptargrep	\
		..\utils\zipdetails	\
		..\utils\shasum		\
		..\utils\instmodsh	\
		..\utils\json_pp	\
		bin\exetype.pl		\
		bin\runperl.pl		\
		bin\pl2bat.pl		\
		bin\perlglob.pl		\
		bin\search.pl

ifeq ($(CCTYPE),GCC)

CFGSH_TMPL	= config.gc
CFGH_TMPL	= config_H.gc
PERLIMPLIB	= $(COREDIR)\libperl538$(a)
PERLIMPLIBBASE	= libperl538$(a)
PERLSTATICLIB	= ..\libperl538s$(a)
INT64		= long long

else

CFGSH_TMPL	= config.vc
CFGH_TMPL	= config_H.vc
INT64		= __int64

endif

# makedef.pl must be updated if this changes, and this should normally
# only change when there is an incompatible revision of the public API.
PERLIMPLIB	?= $(COREDIR)\perl538$(a)
PERLIMPLIBBASE	?= perl538$(a)
PERLEXPLIB	?= $(COREDIR)\perl538.exp
PERLSTATICLIB	?= ..\perl538s$(a)
PERLDLL		= ..\perl538.dll
PERLDLLBASE	= perl538.dll

# don't let "gmake -n all" try to run "miniperl.exe make_ext.pl"
PLMAKE		= gmake

XCOPY		= xcopy /f /r /i /d /y
RCOPY		= xcopy /f /r /i /e /d /y
NOOP		= @rem

#first ones are arrange in compile time order for faster parallel building
#see #123867 for details
MICROCORE_SRC	=		\
		..\toke.c	\
		..\regcomp.c	\
		..\regcomp_trie.c	\
		..\regcomp_debug.c	\
		..\regcomp_invlist.c	\
		..\regcomp_study.c	\
		..\regexec.c	\
		..\op.c		\
		..\sv.c		\
		..\pp.c		\
		..\pp_ctl.c	\
		..\pp_sys.c	\
		..\pp_pack.c	\
		..\pp_hot.c	\
		..\gv.c		\
		..\perl.c	\
		..\utf8.c	\
		..\dump.c	\
		..\hv.c		\
		..\av.c		\
		..\builtin.c	\
		..\caretx.c	\
		..\class.c	\
		..\deb.c	\
		..\doio.c	\
		..\doop.c	\
		..\dquote.c	\
		..\globals.c	\
		..\mro_core.c	\
		..\locale.c	\
		..\keywords.c	\
		..\mathoms.c    \
		..\mg.c		\
		..\numeric.c	\
		..\pad.c	\
		..\peep.c	\
		..\perly.c	\
		..\pp_sort.c	\
		..\reentr.c	\
		..\run.c	\
		..\scope.c	\
		..\taint.c	\
		..\time64.c	\
		..\universal.c	\
		..\util.c

EXTRACORE_SRC	+= perllib.c

ifeq ($(PERL_MALLOC),define)
EXTRACORE_SRC	+= ..\malloc.c
endif

EXTRACORE_SRC	+= ..\perlio.c

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

HAVE_COREDIR	= .coreheaders

MICROCORE_OBJ	= $(MICROCORE_SRC:.c=$(o))
CORE_OBJ	= $(MICROCORE_OBJ) $(EXTRACORE_SRC:.c=$(o))
WIN32_OBJ	= $(WIN32_SRC:.c=$(o))

MINICORE_OBJ	= $(subst ..\,mini\,$(MICROCORE_OBJ))	\
		  $(MINIDIR)\miniperlmain$(o)	\
		  $(MINIDIR)\perlio$(o)
MINIWIN32_OBJ	= $(subst .\,mini\,$(WIN32_OBJ))
MINI_OBJ	= $(MINICORE_OBJ) $(MINIWIN32_OBJ)
DLL_OBJ		= $(DYNALOADER)

PERLDLL_OBJ	= $(CORE_OBJ)
PERLEXE_OBJ	= perlmain$(o)
PERLEXEST_OBJ	= perlmainst$(o)

PERLDLL_OBJ	+= $(WIN32_OBJ) $(DLL_OBJ)

ifneq ($(USE_SETARGV),)
SETARGV_OBJ	= setargv$(o)
endif

ifeq ($(ALL_STATIC),define)
# some exclusions, unfortunately, until fixed:
#  - MakeMaker isn't capable enough for SDBM_File (small bug)
STATIC_EXT	= * !SDBM_File
NORMALIZE_STATIC = Normalize_static
else
# specify static extensions here, for example:
# (be sure to include Win32CORE to load Win32 on demand)
#STATIC_EXT	= Win32CORE Cwd Compress/Raw/Zlib
STATIC_EXT	= Win32CORE
NORMALIZE_DYN	= Normalize_dyn
endif

DYNALOADER	= ..\DynaLoader$(o)

# vars must be separated by "\t+~\t+", since we're using the tempfile
# version of config_sh.pl (we were overflowing someone's buffer by
# trying to fit them all on the command line)
#	-- BKS 10-17-1999
CFG_VARS	=					\
		"INST_TOP=$(INST_TOP)"			\
		"INST_VER=$(INST_VER)"			\
		"INST_ARCH=$(INST_ARCH)"		\
		"archname=$(ARCHNAME)"			\
		"cc=$(CC)"				\
		"ld=$(LINK32)"				\
		"ccflags=$(subst ",\",$(EXTRACFLAGS) $(DEFINES) $(BUILDOPT))" \
		"usecplusplus=$(USE_CPLUSPLUS)"		\
		"cf_email=$(EMAIL)"			\
		"d_mymalloc=$(PERL_MALLOC)"		\
		"i_quadmath=$(I_QUADMATH)"		\
		"libs=$(LIBFILES)"			\
		"incpath=$(subst ",\",$(CCINCDIR))"			\
		"libperl=$(subst ",\",$(PERLIMPLIBBASE))"		\
		"libpth=$(subst ",\",$(CCLIBDIR);$(EXTRALIBDIRS))"	\
		"libc=$(LIBC)"				\
		"make=$(PLMAKE)"				\
		"_o=$(o)"				\
		"obj_ext=$(o)"				\
		"_a=$(a)"				\
		"lib_ext=$(a)"				\
		"static_ext=$(STATIC_EXT)"		\
		"usethreads=$(USE_ITHREADS)"		\
		"useithreads=$(USE_ITHREADS)"		\
		"usemultiplicity=$(USE_MULTI)"		\
		"useperlio=$(USE_PERLIO)"		\
		"use64bitint=$(USE_64_BIT_INT)"		\
		"uselongdouble=$(USE_LONG_DOUBLE)"	\
		"usequadmath=$(USE_QUADMATH)"		\
		"usesitecustomize=$(USE_SITECUST)"	\
		"default_inc_excludes_dot=$(DEFAULT_INC_EXCLUDES_DOT)"	\
		"LINK_FLAGS=$(subst ",\",$(LINK_FLAGS))"\
		"optimize=$(subst ",\",$(OPTIMIZE))"	\
		"ARCHPREFIX=$(ARCHPREFIX)"		\
		"WIN64=$(WIN64)"                        \
		"SKIP_CCHOME_CHECK=$(SKIP_CCHOME_CHECK)"

#
# Top targets
#

.PHONY: all info

all : info rebasePE Extensions_nonxs $(PERLSTATIC)

info :
ifeq ($(CCTYPE),GCC)
	@echo # CCTYPE=$(CCTYPE)&& \
	echo # GCCBIN=$(GCCBIN)&& \
	echo # GCCVER=$(GCCVER1).$(GCCVER2).$(GCCVER3)&& \
	echo # GCCTARGET=$(GCCTARGET)&& \
	echo # GCCCROSS=$(GCCCROSS)&& \
	echo # WIN64=$(WIN64)&& \
	echo # ARCHITECTURE=$(ARCHITECTURE)&& \
	echo # ARCHNAME=$(ARCHNAME)&& \
	echo # MAKE=$(PLMAKE)
else
	@echo # CCTYPE=$(CCTYPE)&& \
	echo # WIN64=$(WIN64)&& \
	echo # ARCHITECTURE=$(ARCHITECTURE)&& \
	echo # ARCHNAME=$(ARCHNAME)&& \
	echo # MAKE=$(PLMAKE)
endif
ifeq ($(CCTYPE),)
	@echo Unable to detect gcc and/or architecture!
	@exit 1
endif


..\regcomp$(o) : ..\regcomp.h ..\regcomp_internal.h ..\regnodes.h ..\regcharclass.h

..\regcomp_debug$(o) : ..\regcomp.h ..\regcomp_internal.h ..\regnodes.h ..\regcharclass.h

..\regcomp_invlist$(o) : ..\regcomp.h ..\regcomp_internal.h ..\regnodes.h ..\regcharclass.h

..\regcomp_study$(o) : ..\regcomp.h ..\regcomp_internal.h ..\regnodes.h ..\regcharclass.h

..\regcomp_trie$(o) : ..\regcomp.h ..\regcomp_internal.h ..\regnodes.h ..\regcharclass.h

..\regexec$(o) : ..\regnodes.h ..\regcharclass.h

reonly : ..\regnodes.h .\config.h ..\git_version.h $(GLOBEXE) $(HAVEMINIPERL)\
	$(CONFIGPM) $(UNIDATAFILES) $(PERLEXE)				\
	Extensions_reonly

static: $(PERLEXESTATIC)

#----------------------------------------------------------------

$(GLOBEXE) : perlglob.c
ifeq ($(CCTYPE),GCC)
	$(LINK32) $(EXTRACFLAGS) $(OPTIMIZE) $(BLINK_FLAGS) -mconsole -o $@ perlglob.c $(LIBFILES)
else
	$(CC) $(EXTRACFLAGS) $(OPTIMIZE) $(PDBOUT) -Fe$@ perlglob.c -link $(BLINK_FLAGS) \
	setargv$(o) $(LIBFILES)
endif

..\git_version.h : $(HAVEMINIPERL) ..\make_patchnum.pl
	$(MINIPERL) -I..\lib ..\make_patchnum.pl

# make sure that we recompile perl.c if the git version changes
..\perl$(o) : ..\git_version.h

..\config.sh : $(CFGSH_TMPL) config_sh.PL FindExt.pm $(HAVEMINIPERL)
	$(MINIPERL) -I..\lib config_sh.PL $(CFG_VARS) $(CFGSH_TMPL) > ..\config.sh.tmp
	if exist ..\config.sh del /f ..\config.sh
	rename ..\config.sh.tmp config.sh

# This target is for when changes to the main config.sh happen.
# Edit config.gc, then make perl using GCC in a minimal configuration (i.e.
# with MULTI, ITHREADS, IMP_SYS, LARGE_FILES and PERLIO off), then make
# this target to regenerate config_H.gc.
regen_config_h:
	$(MINIPERL) -I..\lib config_sh.PL --prebuilt $(CFG_VARS) $(CFGSH_TMPL) > ..\config.sh
	$(MINIPERL) -I..\lib ..\configpm --chdir=..
	-del /f $(CFGH_TMPL)
	-$(MINIPERL) -I..\lib config_h.PL "ARCHPREFIX=$(ARCHPREFIX)"
	rename config.h $(CFGH_TMPL)

$(CONFIGPM): ..\config.sh config_h.PL ..\git_version.h
	$(MINIPERL) -I..\lib ..\configpm --chdir=..
	-$(MINIPERL) -I..\lib config_h.PL "ARCHPREFIX=$(ARCHPREFIX)"

.\config.h : $(CONFIGPM)

# See the comment in Makefile.SH explaining this seemingly cranky ordering
..\lib\buildcustomize.pl : $(MINI_OBJ) ..\write_buildcustomize.pl
ifeq ($(CCTYPE),GCC)
	$(LINK32) -mconsole -o $(MINIPERL) $(BLINK_FLAGS) $(MINI_OBJ) $(LIBFILES)
else
	$(LINK32) -out:$(MINIPERL) $(BLINK_FLAGS) \
	    $(DELAYLOAD) $(MINIDELAYLOAD) $(LIBFILES) $(MINI_OBJ)

endif
	$(MINIPERL) -I..\lib -f ..\write_buildcustomize.pl ..

#convinence target, get a working miniperl
mp : $(CONFIGPM)

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
# MINIDIR generates config.h so miniperl.exe is not rebuilt when the 2nd
# config.h is generated in CONFIGPM target, see also the comments for $(MINI_OBJ).
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
	echo #undef Gconvert&& \
	echo #undef HAS_FREXPL&& \
	echo #undef HAS_ISNANL&& \
	echo #undef HAS_MODFL&& \
	echo #undef HAS_MODFL_PROTO&& \
	echo #undef HAS_SQRTL&& \
	echo #undef HAS_STRTOLD&& \
	echo #undef PERL_PRIfldbl&& \
	echo #undef PERL_PRIgldbl&& \
	echo #undef PERL_PRIeldbl&& \
	echo #undef PERL_SCNfldbl&& \
	echo #undef NVTYPE&& \
	echo #undef NVSIZE&& \
	echo #undef LONG_DOUBLESIZE&& \
	echo #undef NV_OVERFLOWS_INTEGERS_AT&& \
	echo #undef NVef&& \
	echo #undef NVff&& \
	echo #undef NVgf&& \
	echo #undef USE_LONG_DOUBLE&& \
	echo #undef I_QUADMATH&& \
	echo #undef USE_QUADMATH&& \
	echo #undef USE_CPLUSPLUS)>> config.h
ifneq ($(CCTYPE),MSVC120)
	@(echo #undef FILE_ptr&& \
	echo #undef FILE_cnt&& \
	echo #undef FILE_base&& \
	echo #undef FILE_bufsiz&& \
	echo #define FILE_ptr^(fp^) PERLIO_FILE_ptr^(fp^)&& \
	echo #define FILE_cnt^(fp^) PERLIO_FILE_cnt^(fp^)&& \
	echo #define FILE_base^(fp^) PERLIO_FILE_base^(fp^)&& \
	echo #define FILE_bufsiz^(fp^) ^(PERLIO_FILE_cnt^(fp^) + PERLIO_FILE_ptr^(fp^) - PERLIO_FILE_base^(fp^)^)&& \
	echo #define I_STDBOOL)>> config.h
endif
ifeq ($(WIN64),define)
ifeq ($(CCTYPE),GCC)
	@(echo #define LONG_DOUBLESIZE ^16)>> config.h
else
	@(echo #define LONG_DOUBLESIZE ^8)>> config.h
endif
	@(echo #define PTRSIZE ^8&& \
	echo #define SSize_t $(INT64)&& \
	echo #define HAS_ATOLL&& \
	echo #define HAS_STRTOLL&& \
	echo #define HAS_STRTOULL&& \
	echo #define Size_t_size ^8)>> config.h
else
ifeq ($(CCTYPE),GCC)
	@(echo #define LONG_DOUBLESIZE ^12)>> config.h
else
	@(echo #define LONG_DOUBLESIZE ^8)>> config.h
endif
	@(echo #define PTRSIZE ^4&& \
	echo #define SSize_t int&& \
	echo #undef HAS_ATOLL&& \
	echo #undef HAS_STRTOLL&& \
	echo #undef HAS_STRTOULL&& \
	echo #define Size_t_size ^4)>> config.h
endif
ifeq ($(USE_64_BIT_INT),define)
	@(echo #define IVTYPE $(INT64)&& \
	echo #define UVTYPE unsigned $(INT64)&& \
	echo #define IVSIZE ^8&& \
	echo #define UVSIZE ^8)>> config.h
ifeq ($(USE_LONG_DOUBLE),define)
	@(echo #define NV_PRESERVES_UV&& \
	echo #define NV_PRESERVES_UV_BITS 64)>> config.h
else ifeq ($(USE_QUADMATH),define)
	@(echo #define NV_PRESERVES_UV&& \
	echo #define NV_PRESERVES_UV_BITS 64)>> config.h
else
	@(echo #undef NV_PRESERVES_UV&& \
	echo #define NV_PRESERVES_UV_BITS 53)>> config.h
endif
	@(echo #define IVdf "I64d"&& \
	echo #define UVuf "I64u"&& \
	echo #define UVof "I64o"&& \
	echo #define UVxf "I64x"&& \
	echo #define UVXf "I64X"&& \
	echo #define USE_64_BIT_INT)>> config.h
else
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
endif
ifeq ($(USE_LONG_DOUBLE),define)
	@(echo #define Gconvert^(x,n,t,b^) sprintf^(^(b^),"%%.*""Lg",^(n^),^(x^)^)&& \
	echo #define HAS_FREXPL&& \
	echo #define HAS_ISNANL&& \
	echo #define HAS_MODFL&& \
	echo #define HAS_MODFL_PROTO&& \
	echo #define HAS_SQRTL&& \
	echo #define HAS_STRTOLD&& \
	echo #define PERL_PRIfldbl "Lf"&& \
	echo #define PERL_PRIgldbl "Lg"&& \
	echo #define PERL_PRIeldbl "Le"&& \
	echo #define PERL_SCNfldbl "Lf"&& \
	echo #define NVTYPE long double&& \
	echo #define NVSIZE LONG_DOUBLESIZE&& \
	echo #define NV_OVERFLOWS_INTEGERS_AT 256.0*256.0*256.0*256.0*256.0*256.0*256.0*2.0*2.0*2.0*2.0*2.0*2.0*2.0*2.0&& \
	echo #define NVef "Le"&& \
	echo #define NVff "Lf"&& \
	echo #define NVgf "Lg"&& \
	echo #undef I_QUADMATH&& \
	echo #undef USE_QUADMATH&& \
	echo #define USE_LONG_DOUBLE)>> config.h
else ifeq ($(USE_QUADMATH),define)
	@(echo #define Gconvert^(x,n,t,b^) sprintf^(^(b^),"%%.*""Lg",^(n^),^(x^)^)&& \
	echo #define HAS_FREXPL&& \
	echo #define HAS_ISNANL&& \
	echo #define HAS_MODFL&& \
	echo #define HAS_MODFL_PROTO&& \
	echo #define HAS_SQRTL&& \
	echo #define HAS_STRTOLD&& \
	echo #define PERL_PRIfldbl "Lf"&& \
	echo #define PERL_PRIgldbl "Lg"&& \
	echo #define PERL_PRIeldbl "Le"&& \
	echo #define PERL_SCNfldbl "Lf"&& \
	echo #define NVTYPE __float128&& \
	echo #define NVSIZE 16&& \
	echo #define NV_OVERFLOWS_INTEGERS_AT 256.0*256.0*256.0*256.0*256.0*256.0*256.0*256.0*256.0*256.0*256.0*256.0*256.0*256.0*2.0&& \
	echo #define NVef "Qe"&& \
	echo #define NVff "Qf"&& \
	echo #define NVgf "Qg"&& \
	echo #undef USE_LONG_DOUBLE&& \
	echo #define I_QUADMATH&& \
	echo #define USE_QUADMATH)>> config.h
else
	@(echo #define Gconvert^(x,n,t,b^) sprintf^(^(b^),"%%.*g",^(n^),^(x^)^)&& \
	echo #undef HAS_FREXPL&& \
	echo #undef HAS_ISNANL&& \
	echo #undef HAS_MODFL&& \
	echo #undef HAS_MODFL_PROTO&& \
	echo #undef HAS_SQRTL&& \
	echo #undef HAS_STRTOLD&& \
	echo #undef PERL_PRIfldbl&& \
	echo #undef PERL_PRIgldbl&& \
	echo #undef PERL_PRIeldbl&& \
	echo #undef PERL_SCNfldbl&& \
	echo #define NVTYPE double&& \
	echo #define NVSIZE ^8&& \
	echo #define NV_OVERFLOWS_INTEGERS_AT 256.0*256.0*256.0*256.0*256.0*256.0*2.0*2.0*2.0*2.0*2.0&& \
	echo #define NVef "e"&& \
	echo #define NVff "f"&& \
	echo #define NVgf "g"&& \
	echo #undef I_QUADMATH&& \
	echo #undef USE_QUADMATH&& \
	echo #undef USE_LONG_DOUBLE)>> config.h
endif
ifeq ($(USE_CPLUSPLUS),define)
	@(echo #define USE_CPLUSPLUS&& \
	echo #endif)>> config.h
else
	@(echo #undef USE_CPLUSPLUS&& \
	echo #endif)>> config.h
endif
#separate line since this is sentinal that this target is done
	@rem. > $(MINIDIR)\.exists

$(MINICORE_OBJ) : $(CORE_NOCFG_H)
	$(CC) -c $(CFLAGS) $(MINIBUILDOPT) -DPERL_EXTERNAL_GLOB -DPERL_IS_MINIPERL $(OBJOUT_FLAG)$@ $(PDBOUT) ..\$(*F).c

$(MINIWIN32_OBJ) : $(CORE_NOCFG_H)
	$(CC) -c $(CFLAGS) $(MINIBUILDOPT) -DPERL_IS_MINIPERL $(OBJOUT_FLAG)$@ $(PDBOUT) $(*F).c

# -DPERL_IMPLICIT_SYS needs C++ for perllib.c
# rules wrapped in .IFs break Win9X build (we end up with unbalanced []s
# unless the .IF is true), so instead we use a .ELSE with the default.
# This is the only file that depends on perlhost.h, vmem.h, and vdir.h

perllib$(o)	: perllib.c perllibst.h .\perlhost.h .\vdir.h .\vmem.h
ifeq ($(USE_IMP_SYS),define)
	$(CC) -c -I. $(CFLAGS_O) $(CXX_FLAG) $(OBJOUT_FLAG)$@ $(PDBOUT) perllib.c
else
	$(CC) -c -I. $(CFLAGS_O) $(OBJOUT_FLAG)$@ $(PDBOUT) perllib.c
endif

# 1. we don't want to rebuild miniperl.exe when config.h changes
# 2. we don't want to rebuild miniperl.exe with non-default config.h
# 3. we can't have miniperl.exe depend on git_version.h, as miniperl creates it
$(MINI_OBJ)	: $(MINIDIR)\.exists $(CORE_NOCFG_H)

$(WIN32_OBJ)	: $(CORE_H)

$(CORE_OBJ)	: $(CORE_H)

$(DLL_OBJ)	: $(CORE_H)


perllibst.h : $(HAVEMINIPERL) $(CONFIGPM) create_perllibst_h.pl
	$(MINIPERL) -I..\lib create_perllibst_h.pl

perldll.def : $(HAVEMINIPERL) $(CONFIGPM) ..\embed.fnc ..\makedef.pl
	$(MINIPERL) -I..\lib -w ..\makedef.pl PLATFORM=win32 $(OPTIMIZE) $(DEFINES) \
	$(BUILDOPT) CCTYPE=$(CCTYPE) TARG_DIR=..\ > perldll.def

$(PERLEXPLIB) : $(PERLIMPLIB)

$(PERLIMPLIB) : perldll.def
ifeq ($(CCTYPE),GCC)
	$(IMPLIB) -k -d perldll.def -D $(PERLDLLBASE) -l $(PERLIMPLIB) -e $(PERLEXPLIB)
else
	lib -def:perldll.def -machine:$(ARCHITECTURE) /OUT:$(PERLIMPLIB)
endif

$(PERLDLL): $(PERLEXPLIB) $(PERLDLL_OBJ) $(PERLDLL_RES) Extensions_static
ifeq ($(CCTYPE),GCC)
	$(LINK32) -shared -o $@ $(BLINK_FLAGS) \
	   $(PERLDLL_OBJ) $(shell type Extensions_static) $(LIBFILES) $(PERLEXPLIB)
else
	$(LINK32) -dll -out:$@ $(BLINK_FLAGS) \
	    @Extensions_static \
	    $(DELAYLOAD) $(LIBFILES) \
	    $(PERLDLL_RES) $(PERLDLL_OBJ) $(PERLEXPLIB)
endif

$(PERLSTATICLIB): $(PERLDLL_OBJ) Extensions_static
ifeq ($(CCTYPE),GCC)
	$(LIB32) $(LIB_FLAGS) $@ $(PERLDLL_OBJ)
	if exist $(STATICDIR) rmdir /s /q $(STATICDIR)
	for %%i in ($(shell type Extensions_static)) do \
		@mkdir $(STATICDIR) && cd $(STATICDIR) && \
		$(ARCHPREFIX)ar x ..\%%i && \
		$(ARCHPREFIX)ar q ..\$@ *$(o) && \
		cd .. && rmdir /s /q $(STATICDIR)
else
	$(LIB32) $(LIB_FLAGS) -out:$@ @Extensions_static \
	    $(PERLDLL_OBJ)
endif
	$(XCOPY) $(PERLSTATICLIB) $(COREDIR)

$(PERLEXE_RES): perlexe.rc $(PERLEXE_MANIFEST) $(PERLEXE_ICO)

$(MINIDIR)\globals$(o) : $(GENERATED_HEADERS)

$(UUDMAP_H) $(MG_DATA_H) : $(BITCOUNT_H)

$(BITCOUNT_H) : $(GENUUDMAP)
	$(GENUUDMAP) $(GENERATED_HEADERS)

$(GENUUDMAP) : ..\mg_raw.h
ifeq ($(CCTYPE),GCC)
	$(LINK32) $(CFLAGS_O) -o..\generate_uudmap.exe ..\generate_uudmap.c \
	$(BLINK_FLAGS) $(LIBFILES)
else
	$(CC) $(CFLAGS_O) $(PDBOUT) -Fe..\generate_uudmap.exe ..\generate_uudmap.c -link $(LIBFILES) $(BLINK_FLAGS)
endif

.PHONY: MakePPPort

MakePPPort : $(HAVEMINIPERL) $(CONFIGPM)
	$(MINIPERL) -I..\lib ..\mkppport

# also known as $(HAVE_COREDIR)
.coreheaders : $(CORE_H)
	$(XCOPY) *.h $(COREDIR)\\*.*
	$(RCOPY) include $(COREDIR)\\*.*
	$(XCOPY) ..\\*.h $(COREDIR)\\*.*
	rem. > $@

perlmain$(o) : runperl.c $(CONFIGPM)
	$(CC) $(subst -DPERLDLL,-UPERLDLL,$(CFLAGS_O)) $(OBJOUT_FLAG)$@ $(PDBOUT) -c runperl.c

perlmainst$(o) : runperl.c $(CONFIGPM)
	$(CC) $(CFLAGS_O) $(OBJOUT_FLAG)$@ $(PDBOUT) -c runperl.c

$(PERLEXE): $(CONFIGPM) $(PERLEXE_OBJ) $(PERLEXE_RES) $(PERLIMPLIB)
ifeq ($(CCTYPE),GCC)
	$(LINK32) -mconsole -o $@ $(BLINK_FLAGS)  \
	    $(PERLEXE_OBJ) $(PERLEXE_RES) $(PERLIMPLIB) $(LIBFILES)
else
	$(LINK32) -out:$@ $(BLINK_FLAGS) \
	    $(PERLEXE_OBJ) $(PERLEXE_RES) $(PERLIMPLIB) $(LIBFILES) $(SETARGV_OBJ)
endif
	copy $(PERLEXE) $(WPERLEXE)
	$(MINIPERL) -I..\lib bin\exetype.pl $(WPERLEXE) WINDOWS

$(PERLEXESTATIC): $(PERLSTATICLIB) $(CONFIGPM) $(PERLEXEST_OBJ) $(PERLEXE_RES)
ifeq ($(CCTYPE),GCC)
	$(LINK32) -mconsole -o $@ $(BLINK_FLAGS) \
	    $(PERLEXEST_OBJ) $(PERLEXE_RES) $(PERLSTATICLIB) $(LIBFILES)
else
	$(LINK32) -out:$@ $(BLINK_FLAGS) \
	    $(PERLEXEST_OBJ) $(PERLEXE_RES) $(PERLSTATICLIB) $(LIBFILES) $(SETARGV_OBJ)
endif

#-------------------------------------------------------------------------------
# There's no direct way to mark a dependency on
# DynaLoader.pm, so this will have to do

#most of deps of this target are in DYNALOADER and therefore omitted here
Extensions : $(PERLDEP) $(DYNALOADER) Extension_lib $(GLOBEXE) MakePPPort
	$(MINIPERL) -I..\lib ..\make_ext.pl "MAKE=$(PLMAKE)" --dir=$(CPANDIR) --dir=$(DISTDIR) --dir=$(EXTDIR) --dynamic !Unicode/Normalize

Normalize_static : $(CONFIGPM) $(GLOBEXE) $(HAVE_COREDIR) $(UNIDATAFILES)
	$(MINIPERL) -I..\lib ..\make_ext.pl "MAKE=$(PLMAKE)" --dir=$(CPANDIR) --dir=$(DISTDIR) --dir=$(EXTDIR) --static +Unicode/Normalize

Normalize_dyn : $(PERLDEP) $(DYNALOADER) $(GLOBEXE) $(UNIDATAFILES)
	$(MINIPERL) -I..\lib ..\make_ext.pl "MAKE=$(PLMAKE)" --dir=$(CPANDIR) --dir=$(DISTDIR) --dir=$(EXTDIR) --dynamic +Unicode/Normalize

Extensions_reonly : $(PERLDEP) $(DYNALOADER)
	$(MINIPERL) -I..\lib ..\make_ext.pl "MAKE=$(PLMAKE)" --dir=$(CPANDIR) --dir=$(DISTDIR) --dir=$(EXTDIR) --dynamic +re

Exts_static_general : ..\make_ext.pl $(CONFIGPM) Extension_lib $(GLOBEXE) $(HAVE_COREDIR) MakePPPort
	$(MINIPERL) -I..\lib ..\make_ext.pl "MAKE=$(PLMAKE)" --dir=$(CPANDIR) --dir=$(DISTDIR) --dir=$(EXTDIR) --static !Unicode/Normalize

Extensions_static : list_static_libs.pl Exts_static_general $(NORMALIZE_STATIC)
	$(MINIPERL) -I..\lib list_static_libs.pl > Extensions_static

Extensions_nonxs : ..\make_ext.pl ..\pod\perlfunc.pod $(CONFIGPM) $(GLOBEXE)
	$(MINIPERL) -I..\lib ..\make_ext.pl "MAKE=$(PLMAKE)" --dir=$(CPANDIR) --dir=$(DISTDIR) --dir=$(EXTDIR) --nonxs !libs

Extension_lib : ..\make_ext.pl $(CONFIGPM)
	$(MINIPERL) -I..\lib ..\make_ext.pl "MAKE=$(PLMAKE)" --dir=$(CPANDIR) --dir=$(DISTDIR) --dir=$(EXTDIR) lib

#lib must be built, it can't be buildcustomize.pl-ed, and is required for XS building
$(DYNALOADER) : ..\make_ext.pl $(CONFIGPM) $(HAVE_COREDIR)
	$(MINIPERL) -I..\lib ..\make_ext.pl "MAKE=$(PLMAKE)" --dir=$(EXTDIR) --dir=$(DISTDIR) --dynaloader

Extensions_clean :
	-if exist $(MINIPERL) $(MINIPERL) -I..\lib ..\make_ext.pl "MAKE=$(PLMAKE)" --dir=$(CPANDIR) --dir=$(DISTDIR) --dir=$(EXTDIR) --all --target=clean

Extensions_realclean :
	-if exist $(MINIPERL) $(MINIPERL) -I..\lib ..\make_ext.pl "MAKE=$(PLMAKE)" --dir=$(CPANDIR) --dir=$(DISTDIR) --dir=$(EXTDIR) --all --target=realclean

# all PE files need to be built by the time this target runs, PP files can still
# be running in parallel like UNIDATAFILES, this target a placeholder for the
# future
ifeq ($(PERLSTATIC),static)
rebasePE : Extensions $(PERLDLL) $(PERLEXE) $(PERLEXESTATIC)
else
rebasePE : Extensions $(PERLDLL) $(NORMALIZE_DYN) $(PERLEXE)
endif
	$(NOOP)

#-------------------------------------------------------------------------------

doc: $(PERLEXE) $(PERLDLL) ..\pod\perltoc.pod
	$(PERLEXE) -I..\lib ..\installhtml --podroot=.. --htmldir=$(HTMLDIR) \
	    --podpath=pod:lib:utils --htmlroot="file://$(subst :,|,$(INST_HTML))"\
	    --recurse

..\utils\Makefile: $(CONFIGPM) ..\utils\Makefile.PL
	$(MINIPERL) -I..\lib ..\utils\Makefile.PL ..

# Note that this next section is parsed (and regenerated) by pod/buildtoc
# so please check that script before making structural changes here
utils: $(HAVEMINIPERL) ..\utils\Makefile
	cd ..\utils && $(PLMAKE) PERL=$(MINIPERL)
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
	$(MINIPERL) -I..\lib $(PL2BAT) $(UTILS)
	$(MINIPERL) -I..\lib ..\autodoc.pl ..
	$(MINIPERL) -I..\lib ..\pod\perlmodlib.PL -q ..

..\pod\perltoc.pod: $(PERLEXE) $(PERLDLL) Extensions Extensions_nonxs $(NORMALIZE_DYN) utils
	$(PERLEXE) -f ..\pod\buildtoc -q

# Note that the pod cleanup in this next section is parsed (and regenerated
# by pod/buildtoc so please check that script before making changes here

distclean: realclean
	-del /f $(MINIPERL) $(PERLEXE) $(PERLDLL) $(GLOBEXE) \
		$(PERLIMPLIB) ..\miniperl$(a) $(PERLEXESTATIC) $(PERLSTATICLIB)
	-del /f $(LIBDIR)\Encode.pm $(LIBDIR)\encoding.pm $(LIBDIR)\Errno.pm
	-del /f $(LIBDIR)\Config.pod $(LIBDIR)\POSIX.pod $(LIBDIR)\threads.pm
	-del /f $(LIBDIR)\.exists $(LIBDIR)\attributes.pm $(LIBDIR)\DynaLoader.pm
	-del /f $(LIBDIR)\Fcntl.pm $(LIBDIR)\IO.pm $(LIBDIR)\Opcode.pm
	-del /f $(LIBDIR)\ops.pm $(LIBDIR)\Safe.pm
	-del /f $(LIBDIR)\SDBM_File.pm $(LIBDIR)\Socket.pm $(LIBDIR)\POSIX.pm
	-del /f $(LIBDIR)\B.pm $(LIBDIR)\O.pm $(LIBDIR)\re.pm
	-del /f $(LIBDIR)\File\Glob.pm
	-del /f $(LIBDIR)\Sys\Hostname.pm
	-del /f $(LIBDIR)\Time\HiRes.pm
	-del /f $(LIBDIR)\Unicode\Normalize.pm
	-del /f $(LIBDIR)\Math\BigInt\FastCalc.pm
	-del /f $(LIBDIR)\Storable.pm $(LIBDIR)\Storable\Limit.pm
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
	    perldoc perlivp libnetcfg enc2xs encguess piconv cpan *.bat \
	    xsubpp pod2html instmodsh json_pp prove ptar ptardiff ptargrep shasum corelist zipdetails
	-del /f ..\config.sh perlmain.c dlutils.c config.h.new \
	    perlmainst.c
	-del /f $(CONFIGPM)
	-del /f ..\lib\Config_git.pl
	-del /f "bin\*.bat"
	-del /f perllibst.h
	-del /f $(PERLEXE_RES) perl.base
	-cd .. && del /s *$(a) *.map *.pdb *.ilk *.bs *$(o) .exists pm_to_blib ppport.h
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
	if exist $(WPERLEXE) $(XCOPY) $(WPERLEXE) $(INST_BIN)\$(NULL)
	if exist $(PERLEXESTATIC) $(XCOPY) $(PERLEXESTATIC) $(INST_BIN)\$(NULL)
	$(XCOPY) $(GLOBEXE) $(INST_BIN)\$(NULL)
	if exist ..\perl*.pdb $(XCOPY) ..\perl*.pdb $(INST_BIN)\$(NULL)
	$(XCOPY) "bin\*.bat" $(INST_SCRIPT)\$(NULL)

installhtml : doc
	$(RCOPY) $(HTMLDIR)\*.* $(INST_HTML)\$(NULL)

inst_lib : $(CONFIGPM)
	$(RCOPY) ..\lib $(INST_LIB)\$(NULL)

$(UNIDATAFILES) : ..\pod\perluniprops.pod

..\pod\perluniprops.pod: ..\lib\unicore\mktables $(CONFIGPM)
	$(MINIPERL) -I..\lib ..\lib\unicore\mktables -C ..\lib\unicore -P ..\pod -maketest -makelist -p

minitest : $(HAVEMINIPERL) $(GLOBEXE) $(CONFIGPM) $(UNIDATAFILES) utils
	$(XCOPY) $(MINIPERL) ..\t\$(NULL)
	if exist ..\t\perl.exe del /f ..\t\perl.exe
	rename ..\t\miniperl.exe perl.exe
	$(XCOPY) $(GLOBEXE) ..\t\$(NULL)
	attrib -r "..\t\*.*"
	cd ..\t && \
	$(MINIPERL) -I..\lib harness base/*.t comp/*.t cmd/*.t io/*.t opbasic/*.t op/*.t pragma/*.t

test-prep : all utils ..\pod\perltoc.pod $(TESTPREPGCC)
	$(XCOPY) $(PERLEXE) ..\t\$(NULL)
	$(XCOPY) $(PERLDLL) ..\t\$(NULL)
	$(XCOPY) $(GLOBEXE) ..\t\$(NULL)
# If building with gcc versions 4.x.x or greater, then
# the GCC helper DLL will also need copied to the test directory.
# The name of the dll can change, depending upon which vendor has supplied
# your compiler, and upon the values of "x".
# libstdc++-6.dll is copied if it exists as it, too, may then be needed.
# Without this copying, the op/taint.t test script will fail.

ifeq ($(CCTYPE),GCC)

test-prep-gcc :
	if exist $(CCDLLDIR)\libgcc_s_seh-1.dll $(XCOPY) $(CCDLLDIR)\libgcc_s_seh-1.dll ..\t\$(NULL)
	if exist $(CCDLLDIR)\libgcc_s_sjlj-1.dll $(XCOPY) $(CCDLLDIR)\libgcc_s_sjlj-1.dll ..\t\$(NULL)
	if exist $(CCDLLDIR)\libgcc_s_dw2-1.dll $(XCOPY) $(CCDLLDIR)\libgcc_s_dw2-1.dll ..\t\$(NULL)
	if exist $(CCDLLDIR)\libstdc++-6.dll $(XCOPY) $(CCDLLDIR)\libstdc++-6.dll ..\t\$(NULL)
	if exist $(CCDLLDIR)\libwinpthread-1.dll $(XCOPY) $(CCDLLDIR)\libwinpthread-1.dll ..\t\$(NULL)
	if exist $(CCDLLDIR)\libquadmath-0.dll $(XCOPY) $(CCDLLDIR)\libquadmath-0.dll ..\t\$(NULL)

endif

test : test-prep
	set PERL_STATIC_EXT=$(STATIC_EXT) && \
	    cd ..\t && perl.exe harness $(TEST_ARGS) $(TEST_SWITCHES) $(TEST_FILES)

test_porting : test-prep
	set PERL_STATIC_EXT=$(STATIC_EXT) && \
	    cd ..\t && perl.exe harness $(TEST_ARGS) $(TEST_SWITCHES) porting\*.t ..\lib\diagnostics.t

test-reonly : reonly utils
	$(XCOPY) $(PERLEXE) ..\t\$(NULL)
	$(XCOPY) $(PERLDLL) ..\t\$(NULL)
	$(XCOPY) $(GLOBEXE) ..\t\$(NULL)
	cd ..\t && perl.exe harness $(OPT) -re \bpat\\/ $(EXTRA)

regen :
	cd .. && regen.pl

test-notty : test-prep
	set PERL_STATIC_EXT=$(STATIC_EXT) && \
	    set PERL_SKIP_TTY_TEST=1 && \
	    cd ..\t && perl.exe harness $(TEST_ARGS) $(TEST_SWITCHES) $(TEST_FILES)

_test :
	$(XCOPY) $(PERLEXE) ..\t\$(NULL)
	$(XCOPY) $(PERLDLL) ..\t\$(NULL)
	$(XCOPY) $(GLOBEXE) ..\t\$(NULL)
	set PERL_STATIC_EXT=$(STATIC_EXT) && \
	    cd ..\t && perl.exe harness $(TEST_ARGS) $(TEST_SWITCHES) $(TEST_FILES)

_clean :
	-@erase miniperlmain$(o)
	-@erase $(MINIPERL)
	-@erase perlglob$(o)
	-@erase perlmain$(o)
	-@erase perlmainst$(o)
	-@erase /f config.h
	-@erase /f ..\git_version.h
	-@erase $(GLOBEXE)
	-@erase $(PERLEXE)
	-@erase $(WPERLEXE)
	-@erase $(PERLEXESTATIC)
	-@erase $(PERLSTATICLIB)
	-@erase $(PERLDLL)
	-@erase $(CORE_OBJ)
	-@erase $(GENUUDMAP) $(GENUUDMAP_OBJ) $(GENERATED_HEADERS)
	-@erase .coreheaders
	-if exist $(MINIDIR) rmdir /s /q $(MINIDIR)
	-if exist $(UNIDATADIR1) rmdir /s /q $(UNIDATADIR1)
	-if exist $(UNIDATADIR2) rmdir /s /q $(UNIDATADIR2)
	-@erase $(UNIDATAFILES)
	-@erase $(WIN32_OBJ)
	-@erase $(DLL_OBJ)
	-@erase ..\*$(o) ..\*$(a) ..\*.exp *$(o) *$(a) *.exp *.res
	-@erase ..\t\*.exe ..\t\*.dll ..\t\*.bat
	-@erase *.ilk
	-@erase *.pdb ..\*.pdb
	-@erase Extensions_static

clean : Extensions_clean _clean

realclean : Extensions_realclean _clean

# Handy way to run perlbug -ok without having to install and run the
# installed perlbug. We don't re-run the tests here - we trust the user.
# Please *don't* use this unless all tests pass.
# If you want to report test failures, use "gmake nok" instead.
ok: utils $(PERLEXE) $(PERLDLL) Extensions_nonxs Extensions
	$(PERLEXE) ..\utils\perlbug -ok -s "(UNINSTALLED)"

okfile: utils $(PERLEXE) $(PERLDLL) Extensions_nonxs Extensions
	$(PERLEXE) ..\utils\perlbug -ok -s "(UNINSTALLED)" -F perl.ok

nok: utils $(PERLEXE) $(PERLDLL) Extensions_nonxs Extensions
	$(PERLEXE) ..\utils\perlbug -nok -s "(UNINSTALLED)"

nokfile: utils $(PERLEXE) $(PERLDLL) Extensions_nonxs Extensions
	$(PERLEXE) ..\utils\perlbug -nok -s "(UNINSTALLED)" -F perl.nok
